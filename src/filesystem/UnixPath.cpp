/**
 *   @file: UnixPath.cpp
 *
 *   @date: Aug 16, 2017
 * @author: Mateusz Midor
 */

#include "UnixPath.h"

using namespace kstd;
namespace filesystem {

UnixPath::UnixPath(const char path[]) :
    path(path) {

}

UnixPath::UnixPath(const string& path) :
    path(path) {
}

bool UnixPath::is_valid_absolute_path() const {
    if (path.empty())
        return false;

    if (path.front() != '/')
        return false;

    return true;
}

bool UnixPath::is_root_path() const {
    return path == "/";
}

UnixPath::operator string() const {
    return path;
}

/**
 * @brief   Normalize complex unix-like path eg. /user/home/../data/./   into /user/home/data
 *          In generale, this method takes care of '.' and '..' in the path
 */
UnixPath UnixPath::normalize() const {
    if (path.empty())
        return UnixPath("");

    auto segments = split_string<vector<string>>(path, '/');
    vector<string> out;

    for (const auto& s : segments) {
        if (s == "") {}
        else if (s == ".") {}
        else if (s == "..") {
            if (out.empty())
                return UnixPath("");
            else
                out.pop_back();
        } else
            out.push_back(s);
    }

    // keep starting slash if present in source
    if (path[0] == '/')
        return format("/%", join_string("/", out));
    else
        return join_string("/", out);
}

string UnixPath::extract_directory() const {
    auto pivot = path.rfind('/');
    return path.substr(0, pivot+1);
}

string UnixPath::extract_file_name() const {
    auto pivot = path.rfind('/');
    return path.substr(pivot+1, path.size());
}
} /* namespace filesystem */
