/**
 *   @file: Vfs.h
 *
 *   @date: Oct 22, 2017
 * @author: Mateusz Midor
 */


#ifndef SRC_MIDDLESPACE_VFS_H_
#define SRC_MIDDLESPACE_VFS_H_

#include "types.h"

namespace middlespace {

/**
 * @brief   Structure representing a single element in a virtual filesystem
 */
struct VfsEntry {
    bool    is_directory;
    u32     size;
    char    name[256];
};

}



#endif /* SRC_MIDDLESPACE_VFS_H_ */
