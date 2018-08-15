/**
 *   @file: Fat32Utils.h
 *
 *   @date: Jul 12, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_FAT32_FAT32UTILS_H_
#define SRC_FILESYSTEM_FAT32_FAT32UTILS_H_

#include "types.h"
#include "String.h"

namespace filesystem {

class Fat32Utils {
public:
    static bool fits_in_8_3(const cstd::string& filename);
    static cstd::string make_8_3_filename(const cstd::string& filename, u8 seq_num = 0);
    static void make_8_3_filename(const cstd::string& filename, cstd::string& name, cstd::string& ext, u8 seq_num = 0);
    static void make_8_3_space_padded_filename(const cstd::string& filename, cstd::string& name, cstd::string& ext);
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_FAT32_FAT32UTILS_H_ */
