#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utility.h"
#include "builtin_cmd.h"
#include "jobs.h"
#include "proc.h"
#include "env.h"

using namespace std;

#define argc argv.size()

extern jobs_t job_list;
extern env_t env;
map<string, int (*)(const vector<string>&)> func_map;
char cwd[200];

const char* manual = "myshell用户手册\n\
SYNOPSIS\n\t$ mysh [file]\n\
\tmyshell是一个命令解释程序，可以执行从标准输入或者文件中读取的命令。\n\
COPYRIGHT\n\t作者：潘致远，浙江大学\n\
INVOCATION\n\t在终端中打开mysh可执行文件，如果在命令行参数中指定了脚本文件，myshell会逐行读取命令；\
如果没有传入额外的命令行参数，那么myshell将从标准输入读取用户输入。当遇到EOF(Ctrl-D)时，mysh程序会停止。\n\
SHELL GRAMMAR\n\tmyshell采用如下语法格式\n\t\tcmd [arg]* [ | cmd [arg]* ]*[[> filename][< filename][>> filename]]* [&]\n\
\t其中，cmd [arg]* 代表命令cmd，附带着0个或更多个参数arg；[ | cmd [arg]* ]* 代表0个或更多个管道子命令。后面跟的是重定向符号及相应的文件名\
。最后是一个可选的 & 符号，代表让myshell在后台执行该命令。管道、重定向和后台任务详见下文。\
如果cmd为myshell的内部命令，该命令将在mysh程序内部执行；如果cmd为外部程序，myshell会在环境变量PATH中寻找该程序并执行。\n\
\t例子:\n\t\tls -l\n\t\tls -l > out\n\t\tawk -f x | sort -u < infile > outfile &\n\
BUILTIN COMMANDS\n\
\tmyshell集成的内部命令有：bg, cd, clr, dir, echo, environ, exec, exit, fg, help, jobs, pwd, set, shift, test, time, umask, unset\n\
\t说明：\n\
\tbg: bg [作业号]?\t将指定或最近的进程转入后台\n\
\tcd: cd [[-][路径]]?\t更改当前工作目录，转移到指定路径，主目录或上一次到达的路径\n\
\tclr: clr\t\t清空终端窗口中的内容\n\
\tdir: dir [路径]?\t显示指定或当前目录下的文件\n\
\techo: echo [参数]\t向标准输出打印输入的参数\n\
\tset/environ: set [[名称] [参数]]?\t显示所有环境变量的值，或者设置、添加环境变量的值\n\
\texec: exec [命令]\tmyshell执行输入的命令，然后退出程序\n\
\texit: exit\t\t退出myshell\n\
\tfg: fg [作业号]?\t将指定或最近的进程转入前台\n\
\thelp: help\t\t显示用户手册\n\
\tjobs: jobs\t\t显示myshell中活跃的进程列表\n\
\tpwd: pwd\t\t显示当前工作目录\n\
\tshift: shift [偏移量] [参数 ...]\t将输入的参数左移若干位后输出\n\
\ttest: test [[-nz 字符串][变量 运算符 变量]]\t判断输入的条件表达式是否为真并返回结果\n\
\ttime: time\t\t显示当前的系统时间\n\
\tumask: umask [掩码]?\t\t显示当前目录的umask，或将其设为新的值\n\
\tunset: unset [名称]\t\t删除指定的环境变量\n\
I/O REDIRECTION\n\
\t> 表示将标准输出重定向到文件（覆盖写），< 表示将标准输入重定向到文件，>> 表示将标准输出重定向到文件（追加写）。重定向之后，myshell将会从文件中读/写，支持任何内部/外部命令的重定向。\n\
\t例子:\n\t\
\tls ~ >> filelist.txt\n\t\
\twc < file1 > file2\n\
PIPELINE\n\
\t管道是用 | 隔开的一系列命令。前一个命令的标准输出与后一个命令的标准输入用UNIX管道相连，即前一个命令的输出将作为后一个命令的输入，这种连接优先于重定向操作。\n\
\tmyshell支持任何内部/外部命令间的管道\n\
\t例子:\n\t\
\tls | grep a | wc\n\
BACKGROUND\n\
\t& 符号代表让myshell在后台执行该命令。命令进入后台后，myshell会立即返回命令提示符，此时可以继续在命令提示符后工作。\
对于后台任务，可以在作业列表(jobs命令)中跟踪，也可以使用fg或bg命令进行操作。\n\
\t例子:\n\t\
\tsleep 20 &\n\
RUNTIME ENVIRONMENT\n\
\tmyshell维护一张环境变量表，用户可以添加、修改、删除非保留变量。一些内部命令依赖环境变量表中的信息。\n\
\t对于用户的输入，路径中的 ~ 字符会被替换为主目录的绝对路径，变量符号(例如$a)会被替换为变量值。\
\n";

int exit(const vector<string>& argv) {
    for (auto job : job_list.jobs) {
        kill(-job.pid, SIGINT);
    }
    exit(0);
    return 1;
}

int clr(const vector<string>& argv) {
    printf("\033c");
    return 1;
}

int echo(const vector<string>& argv) {
    for (int i = 1;i < argv.size();i++) {
        cout << argv[i];
        if (i != argv.size() - 1) cout << " ";
    }
    cout << endl;
    return 1;
}

int pwd(const vector<string>& argv) {
    printf("%s\n", env.get("PWD"));
    return 1;
}

int time(const vector<string>& argv) {
    time_t timep;
    time(&timep);
    printf("%s", ctime(&timep));
    return 1;
}

int dir(const vector<string>& argv) {
    if (argv.size() < 2) {
        eprintf("用法: dir [目录]\n");
        return 1;
    }
    DIR* dirp;
    struct dirent* dp;
    dirp = opendir(argv[1].c_str());
    if (dirp == NULL) {
        eprintf("mysh: dir: %s: %e\n", argv[1].c_str(), errno);
        return 1;
    }
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            if (dp->d_type == DT_DIR)
                printf("\033[1;34m%s\033[0m\n", dp->d_name);
            else
                printf("%s\n", dp->d_name);
        }
    }
    closedir(dirp);
    return 1;
}

int cd(const vector<string>& argv) {
    const char* dst;
    if (argv.size() < 2) {
        dst = getpwuid(getuid())->pw_dir;
    } else if (argv.size() > 2) {
        eprintf("mysh: cd: 参数太多\n");
        return 1;
    } else {
        dst = argv[1].c_str();
        if (strcmp(dst, "-") == 0) {
            dst = env.get("OLDPWD");
            printf("%s\n", dst);
        }
    }
    if (chdir(dst) < 0) {
        eprintf("mysh: cd: %e\n", errno);
    } else {
        getcwd(cwd, 200);
        env.set("OLDPWD", env.get("PWD"));
        env.set("PWD", cwd);
    }
    return 1;
}

int umask(const vector<string>& argv) {
    if (argv.size() == 1) {
        mode_t prev_mask = umask(0);
        umask(prev_mask);
        printf("%04o\n", prev_mask);
        return 1;
    }
    umask(strtol(argv[1].c_str(), NULL, 8));
    return 1;
}

int set(const vector<string>& argv) {
    if (argv.size() == 1) {
        env.print();
    } else if (argv.size() == 3) {
        if (env.add(argv[1], argv[2]) < 0) {
            eprintf("mysh: set: %s: 受保护的环境变量\n", argv[1].c_str());
        }
    } else {
        eprintf("用法: set [名称] [参数]\n");
    }
    return 1;
}

int unset(const vector<string>& argv) {
    if (argv.size() < 2) {
        eprintf("用法: unset [名称]\n");
    } else {
        if (env.remove(argv[1]) < 0) {
            eprintf("mysh: unset: %s: 受保护的环境变量\n", argv[1].c_str());
        }
    }
    return 1;
}

int jobs(const vector<string>& argv) {
    job_list.print();
    return 1;
}

int bg(const vector<string>& argv) {
    int jid = argc < 2 ? job_list.minjid() : atoi(argv[1].c_str());
    job_t job = job_list.get_job_j(jid);
    if (!job.pid) {
        eprintf("mysh: bg: %s: 无此任务\n", argv[1].c_str());
        return 1;
    }
    job_list.set_state(job.pid, BG);
    kill(job.pid, SIGCONT);
    job_list.print(job.pid);
    return 1;
}

int fg(const vector<string>& argv) {
    int jid = argc < 2 ? job_list.minjid() : atoi(argv[1].c_str());
    job_t job = job_list.get_job_j(jid);
    if (!job.pid) {
        eprintf("mysh: fg: %s: 无此任务\n", argv[1].c_str());
        return 1;
    }
    job_list.set_state(job.pid, FG);
    kill(job.pid, SIGCONT);
    job_list.print(job.pid);
    waitfg(job.pid);
    return 1;
}

int exec(const vector<string>& argv) {
    if (argc < 2) {
        eprintf("用法: exec [命令 [参数 ...]] [重定向 ...]\n");
        return 1;
    }
    char* c_args[50] = { 0 };
    for (int i = 1;i < argv.size();i++) {
        c_args[i-1] = new char[50];
        strcpy(c_args[i-1], argv[i].c_str());
    }
    if (execvp(c_args[0], c_args) < 0) {
        eprintf("mysh: %s: %e\n", c_args[0], errno);
        exit(0);
    }
    return 1;
}

int help(const vector<string>& argv) {
    printf("%s", manual);
    return 1;
}

int shift(const vector<string>& argv) {
    if (argc < 3) {
        fprintf(stderr, "用法: shift [count] args...\n");
        return 1;
    }
    for (int i = atoi(argv[1].c_str()) + 2;i < argv.size();i++) {
        printf("%s", argv[i].c_str());
        if (i != argv.size() - 1) printf(" ");
    }
    printf("\n");
    return 1;
}

#define T() printf("true\n")
#define F() printf("false\n")

int test(const vector<string>& argv) {
    switch (argc) {
    case 1:
        T();
        break;
    case 2:
        T();
        break;
    case 3:
        if (argv[1] == "-z") {
            argv[2].empty() ? T() : F();
        } else if (argv[1] == "-n") {
            argv[2].empty() ? F() : T();
        } else {
            fprintf(stderr, "mysh: test: %s: 需要一元表达式\n", argv[1].c_str());
        }
        break;
    case 4:
        if (argv[2] == "-eq") {
            atoi(argv[1].c_str()) == atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "-ne") {
            atoi(argv[1].c_str()) != atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "-gt") {
            atoi(argv[1].c_str()) > atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "-ge") {
            atoi(argv[1].c_str()) >= atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "-lt") {
            atoi(argv[1].c_str()) < atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "-le") {
            atoi(argv[1].c_str()) <= atoi(argv[3].c_str()) ? T() : F();
        } else if (argv[2] == "=") {
            argv[1] == argv[3] ? T() : F();
        } else if (argv[2] == "!=") {
            argv[1] != argv[3] ? T() : F();
        } else {
            eprintf("mysh: test: %s: 需要二元表达式\n", argv[2].c_str());
        }
        break;
    default:
        eprintf("mysh: test: 参数太多\n");
        break;
    }
    return 1;
}

void init_fmap() {
    func_map["bg"] = bg;
    func_map["cd"] = cd;
    func_map["clr"] = clr;
    func_map["dir"] = dir;
    func_map["fg"] = fg;
    func_map["echo"] = echo;
    func_map["environ"] = set; //alias
    func_map["exec"] = exec;
    func_map["exit"] = exit;
    func_map["help"] = help;
    func_map["jobs"] = jobs;
    func_map["pwd"] = pwd;
    func_map["set"] = set;
    func_map["shift"] = shift;
    func_map["test"] = test;
    func_map["time"] = time;
    func_map["umask"] = umask;
    func_map["unset"] = unset;
}