#pragma once
#define PCI_IDS "/usr/share/misc/pci.ids"

void rplc(std::string *s, const std::string &obj, const std::string &subs) {
    for(int i = 0; ; i += subs.length()) {
        i = s->find(obj, i);
        if(i == std::string::npos) break;
        s->erase(i, obj.length());
        s->insert(i, subs);
    }
}

std::string getDevice(std::string hexVendor, std::string hexDevice) {
    std::string vendorName;
    std::ifstream devlist(PCI_IDS);
    std::string line;
    std::cout << "test" << std::endl;
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

std::string parseColors(std::string output) {
    rplc(&output,"{RESET}",RESET);
    rplc(&output,"{BOLD}",BOLD);
    rplc(&output,"{DIM}",DIM);
    rplc(&output,"{RED}",RED);
    rplc(&output,"{GREEN}",GREEN);
    rplc(&output,"{YELLOW}",YELLOW);
    rplc(&output,"{BLUE}",BLUE);
    rplc(&output,"{MAGENTA}",MAGENTA);
    rplc(&output,"{GRAY}",GRAY);
    rplc(&output,"{WHITE}",WHITE);
    rplc(&output,"{BG_RED}",BG_RED);
    rplc(&output,"{BG_GREEN}",BG_GREEN);
    rplc(&output,"{BG_YELLOW}",BG_YELLOW);
    rplc(&output,"{BG_BLUE}",BG_BLUE);
    rplc(&output,"{BG_MAGENTA}",BG_MAGENTA);
    rplc(&output,"{BG_GRAY}",BG_GRAY);
    rplc(&output,"{BG_WHITE}",BG_WHITE);
    return output;
}