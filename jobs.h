#ifndef _JOBS_H
#define _JOBS_H

#include <unistd.h>
#include <list>
#include <string>

#define UNDEFINED 0
#define FG 1
#define BG 2
#define ST 3

struct job_t {
    pid_t pid;
    int jid;
    int state;
    std::string cmd;
};

struct jobs_t {
    std::list<struct job_t> jobs;
    void add(pid_t pid, int state, std::string cmd);
    void remove(pid_t pid);
    void set_state(pid_t pid, int state);
    void print();
    void print(pid_t pid);
    int next_jid = 1;
    int maxjid();
    int minjid();
    int fgpid();
    const job_t get_job(pid_t pid); // return COPY of jobs in job_list
    const job_t get_job_j(int jid);
};

#endif