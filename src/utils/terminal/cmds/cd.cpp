/**
 *   @file: cd.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "cd.h"
#include "kstd.h"
#include "VolumeFat32.h"
#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;

namespace cmds {

filesystem::VolumeFat32* cd::prev_volume = nullptr;
kstd::string cd::prev_cwd = "";


void cd::run() {
    if (env->cmd_args.size() < 2) {
        env->printer->format("cd: please specify path\n");
        return;
    }

    string path = env->cmd_args[1];

    if (path[0] == '-')
        navigate_back();
    else
        navigate_path(path);
}

void cd::navigate_back() {
    // first check if there is a known previous location
    if (prev_volume) {
        std::swap(env->volume, prev_volume);
        std::swap(env->cwd, prev_cwd);
    }
}

void cd::navigate_path(const string& path) {
    // first store the location
    store_last_location();

    // then navigate onward
    if (path.empty())
        cd_root();
    else if (path[0] == '/')
        cd_volume_directory(path);
    else
        cd_directory(path);
}

void cd::store_last_location() {
    prev_volume = env->volume;
    prev_cwd = env->cwd;
}

void cd::cd_root() {
    cd_directory("/");
}

/**
 * @param absolute_path Path starting with volume name like /SYSTEM/HOME/
 */
void cd::cd_volume_directory(const string& absolute_path) {
    string volume;
    string path;
    split_volume_path(absolute_path, volume, path);

    select_volume_by_name(volume);

    if (!path.empty())
        cd_directory(path);
}

void cd::split_volume_path(const string& location, string& volume, string& path) const {
    // change volume
    auto volume_path_separator = location.find('/', 1);
    if (volume_path_separator == string::npos)
        volume_path_separator = location.size();

    volume = location.substr(1, volume_path_separator - 1);

    // change directory
    path = location.substr(volume_path_separator, location.length());
}
/**
 * @param path Path without volume name
 */
void cd::cd_directory(const string& path) {
    string absolute_path = format("%/%", env->cwd, path);
    string normalized_absolute_path = format("/%", normalize_path(absolute_path)); // absolute path must start with "/"
    auto e = env->volume->get_entry(normalized_absolute_path);
    if (!e) {
        env->printer->format("cd: directory '%' doesnt exist\n", normalized_absolute_path);
        return;
    }

    if (!e.is_directory()) {
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

void cd::select_volume_by_name(const kstd::string& name) {
    for (auto& v : env->volumes)
        if (v.get_label() == name) {
            env->cwd = "/";
            env->volume = &v;
            return;
        }

    env->printer->format("cd: no volume named '%'\n", name);
}

} /* namespace cmds */
