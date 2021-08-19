#ifndef _PROC_H
#define _PROC_H

#include <signal.h>
#include <sys/wait.h>

#define INIT_SET(SIG)  sigset_t mask;\
                       sigemptyset(&mask);\
                       sigaddset(&mask, SIG)
#define BLOCK_SET sigprocmask(SIG_BLOCK, &mask, NULL)
#define UNBLOCK_SET sigprocmask(SIG_UNBLOCK, &mask, NULL)


void add_signal(void (*handler)(int), int sig);

void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

void waitfg(pid_t pid);

#endif