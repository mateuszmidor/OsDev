/**
 *   @file: UnixPath.h
 *
 *   @date: Aug 16, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_UNIXPATH_H_
#define SRC_FILESYSTEM_UNIXPATH_H_

#include "String.h"

namespace filesystem {

/**
 * @brief   This class represents unix-like path in filesystem eg. /home/mateusz/Download
 *          It is always in normalized for ie. no /../ in the path
 */
class UnixPath {
public:
    UnixPath(const char path[]);
    UnixPath(const cstd::string& path = "");
    bool is_valid_absolute_path() const;
    bool is_root_path() const;
    operator cstd::string() const;
    cstd::string extract_directory() const;
    cstd::string extract_file_name() const;

private:
    cstd::string normalize(const cstd::string& path) const;
    cstd::string path;

};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_UNIXPATH_H_ */
