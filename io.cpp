#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <string.h>
#include <fcntl.h>
#include "utility.h"
#include "io.h"
#include "env.h"

#ifdef DEBUG
#define DBG_STR "\033[1;31m[myshell]\033[0m "
#else 
#define DBG_STR ""
#endif

extern env_t env;

void put_prompt() {
    char ind = '$';
    char hostname[200];
    struct passwd* pwd = getpwuid(getuid());
    gethostname(hostname, 200);
    if (strcmp(pwd->pw_name, "root") == 0) {
        ind = '#';
    }
    std::string cwd(env.get("PWD"));
    if (cwd.rfind(pwd->pw_dir, 0) == 0) {
        cwd = "~" + cwd.substr(strlen(pwd->pw_dir));
    }
    printf(DBG_STR"\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m%c ", pwd->pw_name, hostname, cwd.c_str(), ind);
}

int open_script(std::fstream& fs, const char* file) {
    fs.open(file, std::ios::in);
    if (!fs) {
        eprintf("mysh: %s: %e\n", file, errno);
        exit(0);
    }
    return 1;
}

int redir_stdout(const char* file,int concat) {
    int fdout = open(file,
        O_WRONLY | O_CREAT | (concat ? O_APPEND : O_TRUNC), 0666);
    if (fdout < 0) {
        eprintf("mysh: %s: %e\n", file, errno);
        return -1;
    }
    dup2(fdout, fileno(stdout));
    close(fdout);
    return 0;
}

int redir_stdin(const char* file) {
    int fdin = open(file, O_RDONLY);
    if (fdin < 0) {
        eprintf("mysh: %s: %e\n", file, errno);
        return -1;
    }
    dup2(fdin, fileno(stdin));
    close(fdin);
    return 0;
}

void set_multipipe(int pipefd[][2], int pos, int begin, int end) {
    if (pos != begin) {
        dup2(pipefd[pos][0], fileno(stdin));
    }
    close(pipefd[pos][0]);
    close(pipefd[pos][1]);
    if (pos != end) {
        dup2(pipefd[pos + 1][1], fileno(stdout));
    }
}