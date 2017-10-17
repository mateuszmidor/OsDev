/**
 *   @file: VfsPseudoEntry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSPSEUDOENTRY_H_
#define SRC_FILESYSTEM_VFSPSEUDOENTRY_H_

#include "kstd.h"
#include "VfsEntry.h"

namespace filesystem {

/**
 * @brief   This class is an artificial Virtual File System entry implementation, only exists in memory. Eg linux /proc
 */
class VfsPseudoEntry: public VfsEntry {
public:
    VfsPseudoEntry(kstd::string name, bool is_dir = false);
    virtual ~VfsPseudoEntry();

    bool is_directory() const override;
    u32 get_size() const override;
    const kstd::string& get_name() const override;
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;

    kstd::string                name;
    u32                         size        = 0;
    bool                        is_dir      = false;
    kstd::vector<VfsEntryPtr>   entries;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSPSEUDOENTRY_H_ */
