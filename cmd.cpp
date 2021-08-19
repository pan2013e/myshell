#include <iostream>
#include <regex>
#include <fcntl.h>
#include <stdlib.h>
#include <pwd.h>
#include "utility.h"
#include "cmd.h"
#include "jobs.h"
#include "proc.h"
#include "env.h"

using namespace std;

extern jobs_t job_list;
extern env_t env;
extern map<string, int (*)(const vector<string>&)> func_map;

int parse_cmd(cmd_t& c) {
    vector<string> argv;
    regex pat(" ");
    sregex_token_iterator p(c.cmd.begin(), c.cmd.end(), pat, -1);
    sregex_token_iterator end;
    while (p != end) {
        string temp = *p;
        if (temp.find('~', 0) == 0 && (temp.size()==1 || temp[1]=='/')) {
            temp = getpwuid(getuid())->pw_dir + temp.substr(1);
        }
        regex var_pat("\\$([a-zA-Z_]+[\\w]*)");
        smatch res;
        if (regex_match(temp, res, var_pat)) {
            string var_name = res[0].str().substr(1);
            if (env.get(var_name))
                temp.replace(temp.find(res[0]), res[0].length(), env.get(var_name));
            else temp = "";
        }
        argv.push_back(temp);
        p++;
    }
    if (argv.back() == "&") {
        argv.pop_back();
        c.bg = 1;
    }
    for (int i = 0;i < argv.size();i++) {
        if (argv[i] == ">") {
            if (i == argv.size() - 1) return -1;
            c.redir.out = argv[i + 1];
            argv[i] = argv[i + 1] = "";
            c.redir.en = 1;
        } else if (argv[i] == ">>") {
            if (i == argv.size() - 1) return -1;
            c.redir.out = argv[i + 1];
            c.redir.out_concat = 1;
            argv[i] = argv[i + 1] = "";
            c.redir.en = 1;
        } else if (argv[i] == "<") {
            if (i == argv.size() - 1) return -1;
            c.redir.in = argv[i + 1];
            argv[i] = argv[i + 1] = "";
            c.redir.en = 1;
        }
    }
    while (argv.back() == "" && c.redir.en) {
        argv.pop_back();
    }
    for (auto &arg : argv) {
        if (arg == "|") {
            c.pipe = 1;
            break;
        }
    }
    if (c.pipe) {
        int head = 0, tail;
        for (tail = 0;tail < argv.size();tail++) {
            if (argv[tail] == "|") {
                vector<string> temp;
                for (int i = head;i < tail;i++) {
                    temp.push_back(argv[i]);
                }
                c.argvs.push_back(temp);
                head = tail + 1;
            }
        }
        vector<string> temp;
        for (int i = head;i < tail;i++) {
            temp.push_back(argv[i]);
        }
        if (!temp.empty())
            c.argvs.push_back(temp);
    } else {
        c.argvs.push_back(argv);
    }
    c.pipe &= (c.argvs.size() > 1);
    return 0;
}

inline int builtin_cmd(const vector<string>& argv) {
    return func_map.find(argv[0]) != func_map.end();
}

void exec_cmd(cmd_t& c) {
    INIT_SET(SIGCHLD);
    int pipefd[MAXPIPE][2];  // 0-read side, 1-write side
    if (c.pipe) {
        for (int i = 0;i < c.argvs.size();i++) {
            if (pipe(pipefd[i]) < 0) {
                eprintf("mysh: %e\n",errno);
                return;
            }  
        }
    }
    int backup_stdin = dup(fileno(stdin));
    int backup_stdout = dup(fileno(stdout));
    for (int i = 0;i < c.argvs.size();i++) {
        vector<string> args = c.argvs[i];
        if (builtin_cmd(args)) {
            if (i == c.argvs.size() - 1 && !c.redir.out.empty()) {
                if (redir_stdout(c.redir.out.c_str(), c.redir.out_concat) < 0) {
                    exit(1);
                }
            }
            if (c.pipe) {
                set_multipipe(pipefd, i, 0, c.argvs.size() - 1);
            }
            func_map[args[0]](args);
            dup2(backup_stdin, fileno(stdin));
            dup2(backup_stdout, fileno(stdout));
        } else {
            pid_t pid;
            char* c_args[MAXARG] = { 0 };
            for (int i = 0;i < args.size();i++) {
                c_args[i] = new char[MAXARG];
                strcpy(c_args[i], args[i].c_str());
            }
            BLOCK_SET;
            if ((pid = fork()) == 0) {
                setpgid(0, 0);
                UNBLOCK_SET;
                if (i == 0 && !c.redir.in.empty()) {
                    if (redir_stdin(c.redir.in.c_str()) < 0) {
                        exit(1);
                    }
                }
                if (i == c.argvs.size() - 1 && !c.redir.out.empty()) {
                    if (redir_stdout(c.redir.out.c_str(), c.redir.out_concat) < 0) {
                        exit(1);
                    }
                }
                if (c.pipe) {
                    set_multipipe(pipefd, i, 0, c.argvs.size() - 1);
                }
                env.add("PARENT", "/home/pzy/mysh");
                if (execvp(c_args[0], c_args) < 0) {
                    eprintf("mysh: %s: %e\n", c_args[0], errno);
                    exit(0);
                }
            }
            close(pipefd[i][0]);
            close(pipefd[i][1]);
            job_list.add(pid, c.bg ? BG : FG, c.cmd);
            UNBLOCK_SET;
            if (!c.bg) {
                waitfg(pid);
            } else {
                job_list.print(pid);
            }
            for (int i = 0;i < args.size();i++) {
                delete[] c_args[i];
            }
        }
    }
}