/**
 *   @file: VfsRamDummyFileEntry.h
 *
 *   @date: Aug 2, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_
#define KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_

namespace filesystem {

class VfsRamDummyFileEntry: public VfsEntry {
public:
    VfsRamDummyFileEntry(const cstd::string name) : name(name) {}
    const cstd::string& get_name() const override   { return name;                  }
    VfsEntryType get_type() const override          { return VfsEntryType::FILE;    }

private:
    cstd::string name;
};

} /* namespace filesystem */

#endif /* KERNEL_SERVICES_FILESYSTEM_RAMFS_VFSRAMDUMMYFILEENTRY_H_ */
