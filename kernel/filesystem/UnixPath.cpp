/**
 *   @file: UnixPath.cpp
 *
 *   @date: Aug 16, 2017
 * @author: Mateusz Midor
 */

#include "UnixPath.h"
#include "StringUtils.h"

using namespace kstd;
namespace filesystem {

UnixPath::UnixPath(const char path[]) :
    path(normalize(path)) {
}

UnixPath::UnixPath(const string& path) :
    path(normalize(path)) {
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
string UnixPath::normalize(const kstd::string& path) const {
    if (path.empty())
        return {};

    auto segments = StringUtils::split_string(path, '/');
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
        return StringUtils::format("/%", StringUtils::join_string("/", out));
    else
        return StringUtils::join_string("/", out);
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
