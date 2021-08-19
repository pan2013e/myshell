#ifndef _ENV_H
#define _ENV_H

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <unistd.h>

class env_t {
public:
    env_t() {
        table["PWD"] = get_current_dir_name();
        table["OLDPWD"] = table["PWD"];
        table["SHELL"] = "/bin/mysh";
    }
    int add(std::string name, std::string val, int en = 0) {
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        if (en || !restricted(name)) {
            table[name] = val;
            return 0;
        }
        return -1;
    }
    int set(std::string name, std::string val) {
        return add(name, val, 1);
    }
    int remove(std::string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        if (!restricted(name)) {
            table.erase(name);
            return 0;
        }
        return -1;
    }
    const char* get(std::string name) const {
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        if (table.find(name) != table.end()) {
            return table.at(name).c_str();
        } else {
            return NULL;
        }
    }
    void print() const {
        for (auto it = table.begin();it != table.end();it++) {
            printf("%s=%s\n", it->first.c_str(),it->second.c_str());
        }
    }
private:
    int restricted(std::string name) const {
        return find(res_list.begin(), res_list.end(), name) != res_list.end();
    }
    std::vector<std::string> res_list = { "PWD","OLDPWD","SHELL","PARENT" };
    std::map<std::string, std::string> table;
};

#endif