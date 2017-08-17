/**
 *   @file: Fat32Utils.h
 *
 *   @date: Jul 12, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_FAT32UTILS_H_
#define SRC_FILESYSTEM_FAT32_FAT32UTILS_H_

#include "kstd.h"

namespace filesystem {

class Fat32Utils {
public:
    static kstd::string make_8_3_filename(const kstd::string& filename);
    static void make_8_3_space_padded_filename(const kstd::string& filename, kstd::string& name, kstd::string& ext);
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32_FAT32UTILS_H_ */
