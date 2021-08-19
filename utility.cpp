#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "utility.h"

int eprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char ch;
    while (ch = *fmt++) {
        if (ch != '%') {
            putc(ch, stderr);
            continue;
        } else {
            switch (*fmt) {
                case 'e': {
                    static locale_t locale;
                    locale = newlocale(LC_ALL_MASK, "", (locale_t)0);
                    int err_code = va_arg(ap, int);
                    fprintf(stderr, "%s", strerror_l(errno, locale));
                    fmt++;
                    break;
                }
                case 's': {
                    char* s = va_arg(ap, char*);
                    while (*s) {
                        putc(*s, stderr);
                        s++;
                    }
                    fmt++;
                    break;
                }
                case 'd': {
                    int d = va_arg(ap, int);
                    fprintf(stderr, "%d", d);
                    fmt++;
                    break;
                }
            }
        }
    }
    va_end(ap);
    return 0;
}

void trim(std::string& s) {
    if (s.empty())
        return;
    for (char & it : s)
        if (it == '\t') it = ' ';
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
    std::string::iterator next;
    for (std::string::iterator it = s.begin();it != s.end();it = next) {
        next = ++it;
        it--;
        if (*it == ' ' && *next == ' ') {
            next = s.erase(it);
        }
    }
}