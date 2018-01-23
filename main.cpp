#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <fstream>
#include <map>

#include "colors.hpp"
#include "utils.hpp"

#include "ini.hpp"
#include "sys.hpp"

std::string getPath() {
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    std::string res = std::string(result);
    size_t lastdir = res.find_last_of("/");
    std::string dir = res.substr(0, lastdir+1);
    return dir;
}

int nLines(std::string s) {
    int n = 1;
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '\n') {
            n++;
        }
    }
    return n;
}

void moveCursor(int x, int y) {
    while (x<0) {
        std::cout << std::endl;
        x++;
    }
    std::ostringstream xs, ys;
    xs << "\033[" << x-1 << "A";
    ys << "\033[50D\033[" << y << "C";
    std::cout << (x > 0 ? xs.str() : "") << (y > 0 ? ys.str() : "");
}

std::string mkBar(int percentage, ini *config, std::string label) {
    int used = config->bars.width * percentage / 100;
    std::string cLabel[2] = {"", ""};
    cLabel[0] = used < label.length() ? label.substr(0, used) : label;
    cLabel[1] = used < label.length() ? label.substr(used) : "";
    used = used >= label.length() ? used - label.length() : 0;
    std::string usedBar = config->colors["used"]+config->colors["t_used"]+cLabel[0]+std::string(used,' ');
    std::string freeBar = config->colors["free"]+config->colors["t_free"]+cLabel[1]+std::string(config->bars.width-used-label.length(),' ');
    return usedBar + freeBar;
}

std::string buildBars(ff_sysinfo *sys, ini *config) {
    std::stringstream body;
    std::string n = "";
    std::string s(config->palette_spaces, ' ');
    for( std::string bar : config->bar_modules ) {
        if (sys->bars.find(bar) != sys->bars.end()) {
            body << n + mkBar(sys->bars[bar], config, config->bars.label[bar]) + RESET;
            n = "\n";
        } else if (bar == "palette") {
            body << n+"\033[40m"+s+"\033[41m"+s+"\033[42m"+s+"\033[43m"+s+"\033[44m"+s+"\033[45m"+s+"\033[46m"+s+"\033[47m"+RESET;
            n = "\n";
        }
    }
    return body.str();
}

std::string buildSys(ff_sysinfo *sys, ini *config) {
    std::stringstream body;
    std::string n = "";
    for( std::string m : config->sys_modules ) {
        if (sys->modules.find(m) != sys->modules.end()) {
            body << n + config->colors["title"] + m + ": " + RESET + config->colors["normal"] + sys->modules[m] + RESET;
            n = "\n";
        }
    }
    return body.str();
}

std::string buildAscii(std::string asciiPath, ini *config) {
    std::stringstream os;
    std::ifstream in(asciiPath, std::ios::in | std::ios::binary);
    std::stringstream rawstream(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()).c_str());
    std::string line;
    while(std::getline(rawstream,line,'\n')) {
        os << std::string(config->offsets.x, ' ') + RESET + line + RESET + "\n";
    }
    std::string output = parseColors(os.str());
    while (nLines(output) < config->offsets.sy+config->sys_modules.size() && nLines(output) < config->offsets.by+config->bar_modules.size() ) {
        output += '\n';
    }
    return output;
}

std::string buildHeader(ff_sysinfo *sys, ini *config) {
    std::string header = config->m_header;
    rplc(&header,"{CPU}",sys->modules["CPU"]);
    rplc(&header,"{GPU}",sys->modules["GPU"]);
    rplc(&header,"{USER}",sys->modules["User"]);
    rplc(&header,"{UPTIME}",sys->modules["Uptime"]);
    rplc(&header,"{DISTRO}",sys->modules["Distro"]);
    rplc(&header,"{HOSTNAME}",sys->modules["Host"]);
    rplc(&header,"{KERNEL}",sys->modules["Kernel"]);
    rplc(&header,"{PACKAGES}",sys->modules["Packages"]);
    rplc(&header,"{PROCESSES}",sys->modules["Processes"]);
    return parseColors(header);
}

int main(int argc, char const *argv[]) {

    std::string path = getPath();

    const std::string configPath = argc > 1 ? argv[1][0] == '/' ? std::string(argv[1]) : std::string(path+std::string(argv[1])) : std::string("");
    ini config(&configPath);
    std::string asciiPath = config.m_asciiart[0] == '/' ? config.m_asciiart : configPath.substr(0,configPath.find_last_of('/')+1)+config.m_asciiart;

    ff_sysinfo sys(&config);

    std::string sys_b = buildSys(&sys, &config);
    std::string asc_b = config.m_asciiart.empty() || !config.modules["ascii"] ? "" : buildAscii(asciiPath, &config);
    std::string bar_b = config.modules["bars"] ? buildBars(&sys, &config) : "";

    std::cout << asc_b;

    int asc_l = nLines(asc_b);

    int l = asc_l - config.offsets.sy;
    int top_l = 0;
    moveCursor(l, config.offsets.sx);
    l = config.offsets.sy;

    std::stringstream sys_bs(sys_b);
    std::string line;
    while(std::getline(sys_bs,line)) {
        moveCursor(0, config.offsets.sx);
        std::cout << RESET << line << RESET << std::endl;
        top_l = ++l > top_l ? l : top_l;
    }

    moveCursor(l - config.offsets.by, config.offsets.bx);
    l = config.offsets.by;

    std::stringstream bar_bs(bar_b);
    line.clear();
    while(std::getline(bar_bs,line)) {
        moveCursor(0, config.offsets.bx);
        std::cout << line << std::endl;
        top_l = ++l > top_l ? l : top_l;
    }

    moveCursor(l - config.offsets.hy, config.offsets.hx);
    l = config.offsets.hy;
    std::cout << RESET << buildHeader(&sys, &config) << RESET << std::endl;

    int toBottom = asc_l > top_l ? asc_l-1 : top_l;
    while(l<=toBottom-3) { // Don't ask me why sub 3
        std::cout << std::endl;
        l++;
    }

    return 0;
}