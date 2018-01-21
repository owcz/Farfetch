#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <algorithm>
#include <dirent.h>

#define PCI_CLASS_DISPLAY "0x030000"

struct ff_sysinfo {
    private:
        struct utsname un;
        struct statvfs dsk;
        struct sysinfo sinf;
        void trim(std::string *str) {
            while (str->find("  ") != std::string::npos) {
                rplc(str, "  ", " ");
            }
        }
        std::string parseSeconds(long seconds) {
            int days = seconds / 86400;
            int hours = (seconds % 86400) / 3600;
            int minutes = ((seconds % 86400) % 3600) / 60;
            std::string t_parsed = (days > 0 ? std::to_string(days) + (days == 1 ? " day, " : " days, ") : "")\
                                    + (hours > 0 ? std::to_string(hours) + (hours == 1 ? " hour and " : " hours and ") : "")\
                                    + std::to_string(minutes) + " mins";
            return t_parsed;
        }
        void mfreq(std::string *cpustr) {
            std::ifstream mfreqfile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
            std::string content((std::istreambuf_iterator<char>(mfreqfile)), std::istreambuf_iterator<char>());
            if (mfreqfile.good()) {
                std::string output = cpustr->substr(0, cpustr->find_last_of("at")+2) + (char)content[0] + (std::string)"." + (char)content[1] + " GHz";
                *cpustr = output;
            }
        }
        std::string npackages(bool cache) {   // Please, do not laugh at me
            struct stat buffer;
            if (!stat("/tmp/farfetch_npackages", &buffer) == 0 || !cache) {
                std::system("bash -c 'packages=0;\
                type -p pacman >/dev/null && packages=\"$((packages+=$(pacman -Qq --color never | wc -l)))\";\
                type -p dpkg >/dev/null && packages=\"$((packages+=$(dpkg --get-selections | grep -cv deinstall$)))\";\
                type -p kpm >/dev/null && packages=\"$((packages+=$(kpm --get-selections | grep -cv deinstall$)))\";\
                type -p pkgtool >/dev/null && packages=\"$((packages+=$(ls -1 /var/log/packages | wc -l)))\";\
                type -p rpm >/dev/null && packages=\"$((packages+=$(rpm -qa | wc -l)))\";\
                type -p pkginfo >/dev/null && packages=\"$((packages+=$(pkginfo -i | wc -l)))\";\
                type -p emerge >/dev/null && packages=\"$((packages+=$(ls -d /var/db/pkg/*/* | wc -l)))\"; echo $packages > /tmp/farfetch_npackages'");
            }
            std::ifstream nr("/tmp/farfetch_npackages");
            std::string line;
            getline(nr, line);
            return line;
        }
        std::string getAttribFromRaw(const char* file, std::string attrib, char sep) {
            std::ifstream raw(file);
            std::string line;
            while (std::getline(raw,line)) {
                if (line.substr(0,attrib.length()) == attrib){
                    return line.substr(line.find(sep)+2);
                }
            }
        }
        std::string getDevice(std::string hexVendor, std::string hexDevice) {
            /* TODO: Either cache or use a better search algorithm */
            std::string vendorName;
            std::ifstream devlist(this->pci_ids);
            std::string line;
            while (std::getline(devlist,line)) {
                if ((char)line[0] != '#' && (char)line[0] != '\t'){
                    if (line.substr(0, 4) == hexVendor){
                        vendorName = " " + line.substr(line.find("  ")+2);
                        while(std::getline(devlist,line)) {
                            if ((char)line[0] == '\t') {
                                if (line.substr(line.find_last_of('\t')+1, 4) == hexDevice) {
                                    return line.substr(line.find("  ")+2);
                                }
                            } else if ((char)line[0] != '#') {
                                return "Unrecognized" + vendorName;
                            }
                        }
                    }
                }
            }
        }
        std::vector<std::string> getDisplayDevices() {
            std::vector<std::string> devices;
            DIR* dir = opendir("/sys/bus/pci/devices");
            struct dirent *ent;
            while (ent = readdir(dir)) {
                if (ent->d_name[0] != '.'){
                    std::string device_path = (std::string)"/sys/bus/pci/devices/"+ent->d_name+"/";
                    std::ifstream device_class((device_path+"class").c_str());
                    std::string c;
                    std::getline(device_class, c);
                    if (c == PCI_CLASS_DISPLAY){
                        std::ifstream device_vendor((device_path+"vendor").c_str());
                        std::ifstream device_device((device_path+"device").c_str());
                        std::string v;
                        std::string d;
                        std::getline(device_vendor, v);
                        std::getline(device_device, d);
                        devices.push_back(getDevice(v.substr(2), d.substr(2)));
                    }
                }
            }
            closedir(dir);
            return devices;
        }
        std::string getDistro() {
            std::string distro;
            std::string fallback;
            if (fileExists("/etc/os-release")) {
                std::ifstream release("/etc/os-release");
                std::string line;
                while (std::getline(release, line) && !line.empty()) {
                    if (line.substr(0,line.find('=')) == "PRETTY_NAME") {
                        distro = line.substr(line.find('=')+1);
                        rplc(&distro,"\"", "");
                    } else if (line.substr(0,line.find('=')) == "NAME") {
                        fallback = line.substr(line.find('=')+1);
                        rplc(&fallback,"\"", "");
                    }
                }
            }
            return distro.empty() ? fallback : distro;
        }
        unsigned long getCachedRam(int mem_unit) {
            std::ifstream meminfo("/proc/meminfo");
            std::string line;
            while (std::getline(meminfo, line)) {
                if (line.substr(0,7) == "Cached:") {
                    rplc(&line, "Cached:", "");
                    rplc(&line, "kB", "");
                    rplc(&line, " ", "");
                    return (unsigned long)std::stoi(line) * 1024;
                }
            }
        }

    public:

        std::map<const std::string, std::string> modules = {
            {"Kernel",      "err"},
            {"Host",        "err"},
            {"User",        "err"},
            {"CPU",         "err"},
            {"Packages",    "err"},
            {"Uptime",      "err"},
            {"GPU",         "err"},
            {"Processes",   "err"},
            {"Distro",      "err"}
        };

        std::map<std::string, int> bars = {
            {"disk",        0},
            {"ram",         0}
        };

        std::string pci_ids;

        std::vector<const char*> pci_ids_v = {
            "/usr/share/hwdata/pci.ids",
            "/usr/share/misc/pci.ids",
            "/var/lib/pciutils/pci.ids",
            "/usr/share/pci.ids"
        };

        ff_sysinfo(ini *config) {

            bool found_pci_ids = false;

            for (const char* pif : pci_ids_v) {
                std::ifstream pif_test(pif);
                if (pif_test.good()) {
                    this->pci_ids = pif;
                    found_pci_ids = true;
                    break;
                }
            }

            char *username;
            username=(char *)malloc(10*sizeof(char));
            username=getlogin();

            std::string cpu_module = getAttribFromRaw("/proc/cpuinfo", "model name", ':');
            rplc(&cpu_module, "(R)", "");
            rplc(&cpu_module, "(r)", "");
            rplc(&cpu_module, "(TM)", "");
            rplc(&cpu_module, "(tm)", "");
            rplc(&cpu_module, "Core", "");
            rplc(&cpu_module, "CPU", "");
            rplc(&cpu_module, "@", "at");
            mfreq(&cpu_module);
            trim(&cpu_module);

            std::vector<std::string> v_gpu = std::find(config->sys_modules.begin(), config->sys_modules.end(), "GPU") != config->sys_modules.end() && found_pci_ids ?\
                                             (std::vector<std::string>)getDisplayDevices() : (std::vector<std::string>){"err"};

            uname(&un);
            this->modules["Kernel"] = this->un.release;
            this->modules["Host"] = this->un.nodename;
            this->modules["CPU"] = cpu_module;
            this->modules["Packages"] = npackages(config->modules["pkgcache"]);
            this->modules["GPU"] = v_gpu[0]; // Temporal first, TODO, multiple GPUs
            this->modules["User"] = username;
            this->modules["Distro"] = getDistro();

            sysinfo(&sinf);
            this->modules["Uptime"] = parseSeconds(sinf.uptime);
            this->modules["Processes"] = std::to_string((int)sinf.procs);

            statvfs(config->bars.disk.c_str(), &dsk);
            this->bars["disk"] = 100 - (unsigned long)this->dsk.f_bavail * 100 / (unsigned long)this->dsk.f_blocks;
            unsigned long cachedram = getCachedRam(sinf.mem_unit);
            this->bars["ram"] = 100 - (uint64_t)(sinf.freeram+sinf.bufferram+cachedram) * 100 / (uint64_t)(sinf.totalram + sinf.totalswap);

        }
};