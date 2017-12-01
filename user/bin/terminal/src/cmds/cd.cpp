/**
 *   @file: cd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cd.h"
#include "ustd.h"
#include "syscalls.h"
#include "Cout.h"
#include "StringUtils.h"

using namespace ustd;

namespace cmds {

ustd::string cd::prev_cwd = "";
char cwd[256];

void cd::run(const CmdArgs& args, bool run_in_bg) {
    string path;
    if (args.size() > 1)
        path = args[1];

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
    syscalls::getcwd(cwd, sizeof(cwd));
    return cwd;
}

void cd::cd_root() {
    cd_directory("/");
}

/**
 * @param path Path without volume name
 */
void cd::cd_directory(const string& path) {
    struct stat s;

    if (syscalls::stat(path.c_str(), &s) < 0) {
        cout::format("cd: directory '%' doesnt exist\n", path);
        return;
    }

    if (!(s.st_mode == S_IFDIR)) {
        cout::format("cd: '%' is not directory\n", path);
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
    auto segments = StringUtils::split_string(path, '/');
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
    return StringUtils::join_string("/", out);
}

} /* namespace cmds */
