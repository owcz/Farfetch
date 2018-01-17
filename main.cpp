#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <map>

#include "colors.hpp"
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

void rplc(std::string *s, const std::string &obj, const std::string &subs) {
    for(int i = 0; ; i += subs.length()) {
        i = s->find(obj, i);
        if(i == std::string::npos) break;
        s->erase(i, obj.length());
        s->insert(i, subs);
    }
}

std::string mkBar(int barWidth, int percentage) {
    int used = barWidth * percentage / 100;
    return std::string(BAR_USED)+std::string(used,' ')+BAR_FREE+std::string(barWidth-used,' ')+RESET;
}

std::string buildBars(ff_sysinfo *sys, ini *config) {
    std::stringstream body;
    std::string n = "";
    std::string s(config->palette_spaces, ' ');
    for( std::string bar : config->bar_modules ) {
        if (sys->bars.find(bar) != sys->bars.end()) {
            body << n + mkBar(config->bars.width, sys->bars[bar]) + RESET TEXT_NORMAL + " " + config->bars.label[bar];
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
            body << n + TEXT_TITLE + m + ": " + TEXT_NORMAL + sys->modules[m] + RESET;
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
        os << std::string(config->offsets.x, ' ') + line + "\n";
    }
    std::string output = os.str();
    rplc(&output,"{RESET}",RESET);
    rplc(&output,"{BOLD}",BOLD);
    rplc(&output,"{DIM}",DIM);
    rplc(&output,"{RED}",RED);
    rplc(&output,"{GREEN}",GREEN);
    rplc(&output,"{YELLOW}",YELLOW);
    rplc(&output,"{BLUE}",BLUE);
    rplc(&output,"{MAGENTA}",MAGENTA);
    while (nLines(output) < config->offsets.sy+config->sys_modules.size() && nLines(output) < config->offsets.by+config->bar_modules.size() ) {
        output += '\n';
    }
    return output;
}

std::string buildHeader(ff_sysinfo *sys, ini *config) {
    std::string header = config->m_header;
    rplc(&header,"{RESET}",RESET);
    rplc(&header,"{BOLD}",BOLD);
    rplc(&header,"{DIM}",DIM);
    rplc(&header,"{RED}",RED);
    rplc(&header,"{GREEN}",GREEN);
    rplc(&header,"{YELLOW}",YELLOW);
    rplc(&header,"{BLUE}",BLUE);
    rplc(&header,"{MAGENTA}",MAGENTA);
    rplc(&header,"{CPU}",sys->modules["CPU"]);
    rplc(&header,"{HOSTNAME}",sys->modules["Host"]);
    rplc(&header,"{KERNEL}",sys->modules["Kernel"]);
    rplc(&header,"{PACKAGES}",sys->modules["Packages"]);
    return header;
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
        std::cout << line << std::endl;
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
    std::cout << buildHeader(&sys, &config) << RESET << std::endl;

    int toBottom = asc_l > top_l ? asc_l-1 : top_l;
    while(l<=toBottom-3) { // Don't ask me why sub 3
        std::cout << std::endl;
        l++;
    }

    return 0;
}