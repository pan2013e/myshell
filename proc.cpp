#include "proc.h"
#include "jobs.h"

extern jobs_t job_list;

void add_signal(void (*handler)(int), int sig) {
    struct sigaction sact, oact;
    sact.sa_handler = handler;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = SA_RESTART;
    sigaction(sig, &sact, &oact);
}

void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
    if (WIFSTOPPED(status)) {
        job_list.set_state(pid, ST);
        job_list.print(pid);
    } else {
        if (job_list.get_job(pid).state == BG) {
            job_list.set_state(pid, UNDEFINED);
            printf("\n");
            job_list.print(pid);
        }
        job_list.remove(pid);
    }
}

void sigint_handler(int sig) {
    pid_t pid = job_list.fgpid();
    if (pid == 0) return;
    printf("\n");
    kill(-pid, SIGINT);
}

void sigtstp_handler(int sig) {
    pid_t pid = job_list.fgpid();
    if (pid == 0) return;
    printf("\n");
    kill(-pid, SIGTSTP);
}

void waitfg(pid_t pid) {
    pid = job_list.fgpid();
    while (pid > 0) {
        // printf("waiting\n");
        // job_list.print();
        pid = job_list.fgpid();
        // sleep(1);
    }
}