/*
 * VfsEntry.h
 *
 *  Created on: Oct 16, 2017
 *      Author: mateusz
 */

#ifndef SRC_FILESYSTEM_VFSENTRY_H_
#define SRC_FILESYSTEM_VFSENTRY_H_

#include <functional>
#include "kstd.h"

namespace filesystem {

// directory enumeration result
enum class VfsEnumerateResult {
    ENUMERATION_FINISHED,   // all entries in directory have been enumerated
    ENUMERATION_STOPPED,    // OnEntryFound callback returned false
    ENUMERATION_CONTINUE,   // more entries to enumerate in following cluster
    ENUMERATION_FAILED      // could not enumerate eg. because entry is not a directory or is not initialized
};

class VfsEntry;
using OnVfsEntryFound = std::function<bool(VfsEntry& e)>;

class VfsEntry {
public:
    VfsEntry();
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry);
    operator bool() const;
    bool operator!() const;

    kstd::string            name;
    bool                    is_dir = false;
    kstd::vector<VfsEntry*> entries;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSENTRY_H_ */
