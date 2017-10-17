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
 * @brief   This class is an artificial filesystem entry, only exists in memory
 */
class VfsPseudoEntry: public VfsEntry {
public:
    VfsPseudoEntry();
    virtual ~VfsPseudoEntry();

    bool is_directory() const override;
    u32 get_size() const override;
    const kstd::string& get_name() const override;
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;
    VfsEntry* clone() const override;

    kstd::string            name;
    u32                     size    = 0;
    bool                    is_dir  = false;
    kstd::vector<VfsEntry*> entries;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSPSEUDOENTRY_H_ */
