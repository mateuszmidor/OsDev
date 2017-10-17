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

/**
 * @brief   This class is an interface for Virtual File System entry (file or directory)
 */
class VfsEntry {
public:
    VfsEntry() {};
    virtual ~VfsEntry() {}

    virtual VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) = 0;
    virtual bool is_directory() const = 0;
    virtual u32 get_size() const = 0;
    virtual const kstd::string& get_name() const = 0;
    virtual VfsEntry* clone() const = 0;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSENTRY_H_ */
