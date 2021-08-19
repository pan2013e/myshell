#ifndef _UTILITY_H
#define _UTILITY_H

#include <string>
#include <stdarg.h>

int eprintf(const char* fmt, ...);

void trim(std::string& s);

#endif