/**
 *   @file: cd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cd.h"
#include "kstd.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;

namespace cmds {

kstd::string cd::prev_cwd = "";


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
        std::swap(env->cwd, prev_cwd);
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
    prev_cwd = env->cwd;
}

void cd::cd_root() {
    cd_directory("/");
}

/**
 * @param path Path without volume name
 */
void cd::cd_directory(const string& path) {
    string absolute_path = make_absolute_filename(path);
    string normalized_absolute_path = format("/%", normalize_path(absolute_path)); // absolute path must start with "/"
    VfsEntryPtr e = env->vfs_manager.get_entry(normalized_absolute_path);
    if (!e) {
        env->printer->format("cd: directory '%' doesnt exist\n", normalized_absolute_path);
        return;
    }

    if (!e->is_directory()) {
        env->printer->format("cd: '%' is not directory\n", normalized_absolute_path);
        return;
    }

    env->cwd = normalized_absolute_path;
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
