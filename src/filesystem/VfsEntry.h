/*
 * VfsEntry.h
 *
 *  Created on: Oct 16, 2017
 *      Author: mateusz
 */

#ifndef SRC_FILESYSTEM_VFSENTRY_H_
#define SRC_FILESYSTEM_VFSENTRY_H_

#include <memory>
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
using VfsEntryPtr = std::shared_ptr<VfsEntry>;
using OnVfsEntryFound = std::function<bool(VfsEntryPtr e)>;

/**
 * @brief   This class is an interface for Virtual File System entry (file or directory)
 */
class VfsEntry {
public:
    VfsEntry() {};
    virtual ~VfsEntry() {}

    // [common interface]
    virtual bool open() = 0;	// start file manipulating session. this can be used to eg. implement exclusive access to a file
    virtual void close() = 0;	// end file manipulating session.
    virtual bool is_directory() const = 0;
    virtual bool is_mount_point() const { return false; }
    virtual const kstd::string& get_name() const = 0;

    // [file interface]
    virtual u32 get_size() const = 0;
    virtual s64 read(void* data, u32 count) = 0;
    virtual s64 write(const void* data, u32 count) = 0;
    virtual bool seek(u32 new_position) = 0;
    virtual bool truncate(u32 new_size) = 0;

    // [directory interface]
    virtual VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) = 0;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSENTRY_H_ */
