#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <algorithm>
#include <dirent.h>
#include <time.h>

#define PCI_IDS "/usr/share/misc/pci.ids"
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
            time_t sec = (int)seconds;
            tm *ts = gmtime(&sec);
            std::string t_parsed = (ts->tm_yday > 0 ? std::to_string(ts->tm_yday) + (ts->tm_yday == 1 ? " day, " : " days, ") : "")\
                                    + (ts->tm_hour > 0 ? std::to_string(ts->tm_hour) + (ts->tm_hour == 1 ? " hour and " : " hours and ") : "")\
                                    + std::to_string(ts->tm_min) + " mins";
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
            std::ifstream devlist(PCI_IDS);
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

    public:

        std::map<const std::string, std::string> modules = {
            {"Kernel",      "err"},
            {"Host",        "err"},
            {"CPU",         "err"},
            {"Packages",    "err"},
            {"Uptime",      "err"},
            {"GPU",         "err"}
        };

        std::map<std::string, int> bars = {
            {"disk",        0},
            {"ram",         0}
        };

        ff_sysinfo(ini *config) {

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

            std::vector<std::string> v_gpu = std::find(config->sys_modules.begin(), config->sys_modules.end(), "GPU") != config->sys_modules.end() ?\
                                             (std::vector<std::string>)getDisplayDevices() : (std::vector<std::string>){"err"};

            uname(&un);
            this->modules["Kernel"] = this->un.release;
            this->modules["Host"] = this->un.nodename;
            this->modules["CPU"] = cpu_module;
            this->modules["Packages"] = npackages(config->modules["pkgcache"]);
            this->modules["GPU"] = v_gpu[0]; // Temporal 1

            sysinfo(&sinf);
            this->modules["Uptime"] = parseSeconds(sinf.uptime);

            statvfs(config->bars.disk.c_str(), &dsk);
            this->bars["disk"] = 100 - (unsigned long)this->dsk.f_bavail * 100 / (unsigned long)this->dsk.f_blocks;
            this->bars["ram"] = 100 - (((uint64_t) sinf.freeram * sinf.mem_unit)/1024)\
                                     * 100 / ((((uint64_t) sinf.totalram * sinf.mem_unit)/1024 + ((uint64_t) sinf.totalswap * sinf.mem_unit)/1024));
        }
};