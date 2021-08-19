#include <iostream>
#include "jobs.h"
#include "proc.h"
#include "cmd.h"
#include "io.h"
#include "env.h"

using namespace std;

env_t env;

int main(int argc, const char* argv[]) {
    init_fmap();
    add_signal(sigchld_handler, SIGCHLD);
    add_signal(sigint_handler, SIGINT);
    add_signal(sigtstp_handler, SIGTSTP);
    fstream fs;
    int rd_file = argc > 1 ? open_script(fs, argv[1]) : 0;
    while (1) {
        string cmd;
        if (!rd_file) {
            put_prompt();
            getline(cin, cmd);
            if (cin.eof()) {
                printf("\n");
                exit(0);
            }
        } else {
            getline(fs, cmd);
            if (fs.eof()) exit(0);
        }
        cmd_t cmd_wrapper(cmd);
        if (!cmd_wrapper.empty() && parse_cmd(cmd_wrapper) >= 0) {
            exec_cmd(cmd_wrapper);
        }
    }
    return 0;
}