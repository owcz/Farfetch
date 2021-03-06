/*
 *  INI parser, kinga garbage but whatever
 *  By Capuno, GPLv3
 *  #include <map> // required
 *  #include <fstream> // required
 */

#include <string>
#include <sstream>

#include <vector>
#include <iterator>

class ini {

    private:

        std::string inifile;
        std::string rawconf;

        inline bool _isInteger(const std::string & s) { //https://stackoverflow.com/a/2845275/6420180
            if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))){
                return false;
            }
            char *p;
            strtol(s.c_str(), &p, 10);
            return *p == 0;
        }

        void setSysLayout(std::string s, char sep, std::vector<std::string> *sys_modules) {
            rplc(&s, " ", "");
            std::stringstream modules(s);
            std::string module;
            while (std::getline(modules, module, sep)) {
                sys_modules->push_back(module);
            }
        }

        std::string _getRawIni() {
            std::stringstream packed;
            std::string output;
            std::ifstream in(this->inifile.c_str(), std::ios::in | std::ios::binary);
            std::stringstream rawstream(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()).c_str());
            std::string line;
            while(std::getline(rawstream,line,'\n')){
                if (!line.empty()) {
                    line.erase(0, line.find_first_not_of(" \t"));
                    line = line.substr(0, line.find(';') != std::string::npos ? (int)line.find(';') : line.length());
                    packed << line.erase(line.find_last_not_of(" \t")+1) + "\n";
                }
            }
            for (int i = 0; i < packed.str().size(); ++i) {
                if (packed.str()[i] != '"') {
                    output += packed.str()[i];
                }
            }
            return output;
        }

        void split(const char* block, const char* value, std::string *holder) {
            std::stringstream rawstream(this->rawconf.c_str());
            std::stringstream block2e;
            block2e << rawstream.str().substr(rawstream.str().find(std::string(std::string("[")+std::string(block)+std::string("]")))+1);
            std::string line;
            while(std::getline(block2e,line,'\n') && line[0] != '['){
                if (line.substr(0, std::string(value).length()+1) == std::string(value)+std::string("=")) {
                    int d = std::string(value).length()+1;
                    line = line.substr(std::string(value).length()+1, line.find(';') != std::string::npos ? (int)line.find(';') - d : line.length());
                    *holder = line.erase(line.find_last_not_of(" \t")+1);
                }
            }
        }

    public:

        struct bar {
            int width = 28;
            std::string disk = "/home";
            std::map<std::string, std::string> label = {
                {"disk",    ""},
                {"ram",     ""}
            };
        } bars;

        struct offset {
            int x = 0;
            int hx = 0;
            int hy = 0;
            int sx = 1;
            int sy = 1;
            int bx = 1;
            int by = 6;
        } offsets;

        std::map<const char*, bool> modules = {
            {"ascii",           true},
            {"bars",            true},
            {"pkgcache",        false}
        };

        std::map<std::string, std::string> colors = {
            {"title",   "{BOLD}"},
            {"normal",  ""},
            {"used",    "{BG_WHITE}"},
            {"free",    "{BG_GRAY}"},
            {"t_used",    "{GRAY}"},
            {"t_free",    "{WHITE}"}
        };

        std::string m_asciiart;
        std::string m_header;

        std::vector<std::string> sys_modules;
        std::vector<std::string> bar_modules;

        int palette_spaces = 4;

        ini(const std::string* inifile) {

            if (!inifile->empty()) {

                this->inifile = *inifile;

                if (!fileExists(this->inifile.c_str())) {
                    std::string err_log = "Config file " + this->inifile + " does not exist!";
                    throw std::runtime_error(err_log.c_str());
                } else {
                    this->rawconf = _getRawIni();

                    std::string buffer;

                    split("bars","width",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->bars.width = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("bars","disk",&buffer);
                    if (!buffer.empty()) {
                        this->bars.disk = buffer;
                    }
                    split("bars","disk_label",&buffer);
                    if (!buffer.empty()) {
                        this->bars.label["disk"] = buffer;
                    }
                    buffer.clear();
                    split("bars","ram_label",&buffer);
                    if (!buffer.empty()) {
                        this->bars.label["ram"] = buffer;
                    }
                    buffer.clear();
                    split("bars","order",&buffer);
                    setSysLayout(!buffer.empty() ? buffer : std::string("disk,palette"), ',', &this->bar_modules);
                    buffer.clear();
                    split("bars","pw",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->palette_spaces = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","x",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.x = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","sx",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.sx = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","sy",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.sy = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","by",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.by = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","bx",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.bx = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","hx",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.hx = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("offsets","hy",&buffer);
                    if (!buffer.empty() && _isInteger(buffer)) {
                        this->offsets.hy = std::stoi(buffer);
                    }
                    buffer.clear();
                    split("modules","ascii",&buffer);
                    if (!buffer.empty()) {
                        std::istringstream(buffer) >> std::boolalpha >> this->modules["ascii"];
                    }
                    buffer.clear();
                    split("modules","bars",&buffer);
                    if (!buffer.empty()) {
                        std::istringstream(buffer) >> std::boolalpha >> this->modules["bars"];
                    }
                    buffer.clear();
                    split("modules","pkgcache",&buffer);
                    if (!buffer.empty()) {
                        std::istringstream(buffer) >> std::boolalpha >> this->modules["pkgcache"];
                    }
                    buffer.clear();
                    split("modules","order",&buffer);
                    setSysLayout(!buffer.empty() ? buffer : std::string("Host,Kernel,CPU"), ',', &this->sys_modules);
                    buffer.clear();
                    split("custom","art",&buffer);
                    if (!buffer.empty()) {
                        this->m_asciiart = buffer;
                    }
                    buffer.clear();
                    split("custom","h",&buffer);
                    if (!buffer.empty()) {
                        this->m_header = buffer;
                    }
                    buffer.clear();
                    split("custom","text_title",&buffer);
                    if (!buffer.empty()) {
                        this->colors["title"] = buffer;
                    }
                    buffer.clear();
                    split("custom","text_normal",&buffer);
                    if (!buffer.empty()) {
                        this->colors["normal"] = buffer;
                    }
                    buffer.clear();
                    split("custom","bar_used",&buffer);
                    if (!buffer.empty()) {
                        this->colors["used"] = buffer;
                    }
                    buffer.clear();
                    split("custom","bar_free",&buffer);
                    if (!buffer.empty()) {
                        this->colors["free"] = buffer;
                    }
                    buffer.clear();
                    split("custom","bar_used_text",&buffer);
                    if (!buffer.empty()) {
                        this->colors["t_used"] = buffer;
                    }
                    buffer.clear();
                    split("custom","bar_free_text",&buffer);
                    if (!buffer.empty()) {
                        this->colors["t_free"] = buffer;
                    }
                    buffer.clear();
                }
            } else {
                setSysLayout(std::string("Host,Kernel,CPU,Packages"), ',', &this->sys_modules);
                setSysLayout(std::string("disk,palette"), ',', &this->bar_modules);
            }
            for (auto const &c : this->colors) {
                this->colors[c.first] = parseColors(c.second);
            }

        }

};