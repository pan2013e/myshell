#include "jobs.h"

jobs_t job_list;
const char* state_txt[] = { "已完成","前台运行","后台运行","已停止" };

void jobs_t::add(pid_t pid, int state, std::string cmd) {
    struct job_t new_job = { pid, next_jid++, state, cmd };
    jobs.push_back(new_job);
}

void jobs_t::remove(pid_t pid) {
    if (pid < 1) return;
    job_t job = get_job(pid);
    for (auto it = jobs.begin();it != jobs.end();it++) {
        if (it->pid == pid) {
            jobs.erase(it);
            break;
        }
    }
    next_jid = maxjid() + 1;
}

void jobs_t::set_state(pid_t pid, int state) {
    for (auto& job : jobs) {
        if (job.pid == pid) {
            job.state = state;
            break;
        }
    }
}

int jobs_t::maxjid() {
    if (jobs.size() == 0) {
        return 0;
    }
    int max = 0;
    for (auto job : jobs) {
        if (job.jid > max) max = job.jid;
    }
    return max;
}

int jobs_t::minjid() {
    if (jobs.size() == 0) {
        return 0;
    }
    int min = 1 << 12;
    for (auto job : jobs) {
        if (job.jid < min) min = job.jid;
    }
    return min;
}

const job_t jobs_t::get_job(pid_t pid) {
    for (auto job : jobs) {
        if (job.pid == pid) {
            return job;
        }
    }
    job_t default_job = { 0,0,UNDEFINED,"" };
    return default_job;
}

const job_t jobs_t::get_job_j(int jid) {
    for (auto job : jobs) {
        if (job.jid == jid) {
            return job;
        }
    }
    job_t default_job = { 0,0,UNDEFINED,"" };
    return default_job;
}

int jobs_t::fgpid() {
    for (auto &job : jobs) {
        if (job.state == FG) {
            return job.pid;
        }
    }
    return 0;
}

void jobs_t::print(pid_t pid) {
    job_t job = get_job(pid);
    printf("[%d] (%d) %s %s\n", job.jid, job.pid, state_txt[job.state], job.cmd.c_str());
}

void jobs_t::print() {
    for (auto &job : jobs) {
        printf("[%d] (%d) %s %s\n", job.jid, job.pid, state_txt[job.state], job.cmd.c_str());
    }
}