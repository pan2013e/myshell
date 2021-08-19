#ifndef _CMD_H
#define _CMD_H

#include <unistd.h>
#include "builtin_cmd.h"
#include "io.h"
#include "utility.h"

#define MAXPIPE 50
#define MAXARG  100

struct cmd_t {
    cmd_t(std::string s) : cmd(s) { trim(cmd); }
    redir_t redir;
    int bg = 0;
    int pipe = 0;
    std::string cmd;
    std::vector<std::vector<std::string>> argvs;
    int empty() { return cmd.empty(); }
};

int parse_cmd(cmd_t& c);
void exec_cmd(cmd_t& c);

#endif