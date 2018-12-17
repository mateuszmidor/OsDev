/**
 *   @file: VfsOpenEntry.cpp
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#include "VfsOpenEntry.h"

namespace filesystem {

VfsOpenEntry::VfsOpenEntry(VfsCachedEntryPtr e, EntryState* s, const OnDestroy& on_destroy) : entry(e), state(s), on_destroy(on_destroy) {
    if (entry)
        entry->open_count++;
}

VfsOpenEntry::~VfsOpenEntry() {
    dispose();
}

VfsOpenEntry::VfsOpenEntry(VfsOpenEntry&& e) {
    entry = std::move(e.entry);
    state = std::move(e.state);
    on_destroy = std::move(e.on_destroy);
}

VfsOpenEntry& VfsOpenEntry::operator=(VfsOpenEntry&& e) {
    dispose();  // this VfsOpenEntry will now hold new payload so dispose old payload
    entry = std::move(e.entry);
    state = std::move(e.state);
    on_destroy = std::move(e.on_destroy);
    return *this;
}

void VfsOpenEntry::dispose() {
    if (entry) {
        entry->open_count--;
        entry->close(state);
        if (on_destroy)
            on_destroy(entry);
    }
}
} /* namespace filesystem */
