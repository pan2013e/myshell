#ifndef _IO_H
#define _IO_H

#include <string>
#include <fstream>

struct redir_t {
    std::string in = "";
    std::string out = "";
    int out_concat = 0;
    int en = 0;
};

void put_prompt();

int open_script(std::fstream&, const char*);

int redir_stdout(const char* file, int concat);
int redir_stdin(const char* file);

void set_multipipe(int pipefd[][2], int pos, int begin, int end);

#endif