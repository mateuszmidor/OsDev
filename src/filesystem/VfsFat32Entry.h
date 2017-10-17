/**
 *   @file: VfsFat32Entry.h
 *
 *   @date: Oct 17, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_FILESYSTEM_VFSFAT32ENTRY_H_
#define SRC_FILESYSTEM_VFSFAT32ENTRY_H_

#include "Fat32Entry.h"
#include "VfsEntry.h"

namespace filesystem {

class VfsFat32Entry: public VfsEntry {
public:
    VfsFat32Entry(const Fat32Entry& e);
    virtual ~VfsFat32Entry();

    bool is_directory() const override;
    u32 get_size() const override;
    const kstd::string& get_name() const override;
    VfsEnumerateResult enumerate_entries(const OnVfsEntryFound& on_entry) override;
    VfsEntry* clone() const override;

    void set_custom_name(const kstd::string& name);

private:
    Fat32Entry      entry;
    kstd::string    name;
};

} /* namespace filesystem */

#endif /* SRC_FILESYSTEM_VFSFAT32ENTRY_H_ */
