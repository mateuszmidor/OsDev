/**
 *   @file: cd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cd.h"
#include "ustd.h"
#include "syscalls.h"

using namespace ustd;

namespace cmds {

ustd::string cd::prev_cwd = "";
const u32 MAX_PATH = 256;
char cwd[MAX_PATH];

void cd::run() {
    string path;
    if (env->cmd_args.size() > 1)
        path = env->cmd_args[1];

    if (path == "-")
        navigate_back();
    else
        navigate_path(path);
}

void cd::navigate_back() {
    // first check if there is a known previous location
    if (!prev_cwd.empty()) {
        string tmp = prev_cwd;
        prev_cwd = get_cwd();
        syscalls::chdir(tmp.c_str());
    }
}

void cd::navigate_path(const string& path) {
    // first store the location
    store_last_location();

    // then navigate onward
    if (path.empty())
        cd_root();
    else
        cd_directory(path);
}

void cd::store_last_location() {
    prev_cwd = get_cwd();
}

string cd::get_cwd() const {
    return syscalls::getcwd(cwd, MAX_PATH);
}

void cd::cd_root() {
    cd_directory("/");
}

/**
 * @param path Path without volume name
 */
void cd::cd_directory(const string& path) {
    struct stat s;

    if (syscalls::stat(path.c_str(), &s) == -1) {
        env->printer->format("cd: directory '%' doesnt exist\n", path);
        return;
    }

    if (!(s.st_mode == S_IFDIR)) {
        env->printer->format("cd: '%' is not directory\n", path);
        return;
    }

    syscalls::chdir(path.c_str());
}

/**
 *
 * @param path  Difficult path like /user/home/../data/./..
 * @return      Normalized path without starting slash
 */
string cd::normalize_path(const string& path) const {
    auto segments = split_string<vector<string>>(path, '/');
    vector<string> out;

    for (const auto& s : segments) {
        if (s == "") {}
        else if (s == ".") {}
        else if (s == "..") {
            if (out.empty())
                return "";
            else
                out.pop_back();
        } else
            out.push_back(s);

    }
    return join_string("/", out);
}

} /* namespace cmds */
