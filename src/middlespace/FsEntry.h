/*
 * FsEntry.h
 *
 *  Created on: Oct 22, 2017
 *      Author: mateusz
 */

#ifndef SRC_MIDDLESPACE_FILESYSTEM_FSENTRY_H_
#define SRC_MIDDLESPACE_FILESYSTEM_FSENTRY_H_

#include "types.h"

namespace middlespace {

struct FsEntry {
    bool    is_directory;
    u32     size;
    char    name[256];
};
}



#endif /* SRC_MIDDLESPACE_FILESYSTEM_FSENTRY_H_ */
