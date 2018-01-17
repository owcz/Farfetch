#pragma once

void rplc(std::string *s, const std::string &obj, const std::string &subs) {
    for(int i = 0; ; i += subs.length()) {
        i = s->find(obj, i);
        if(i == std::string::npos) break;
        s->erase(i, obj.length());
        s->insert(i, subs);
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