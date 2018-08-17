/**
 *   @file: OpenEntry.cpp
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntry.h"

namespace filesystem {

OpenEntry::OpenEntry(VfsCachedEntryPtr e, EntryState* s, const OnDestroy& on_destroy) : entry(e), state(s), on_destroy(on_destroy) {
    if (entry)
        entry->open_count++;
}

OpenEntry::~OpenEntry() {
    dispose();
}

OpenEntry::OpenEntry(OpenEntry&& e) {
    entry = std::move(e.entry);
    state = std::move(e.state);
    on_destroy = std::move(e.on_destroy);
}

OpenEntry& OpenEntry::operator=(OpenEntry&& e) {
    dispose();  // this OpenEntry will now hold new payload so dispose old payload
    entry = std::move(e.entry);
    state = std::move(e.state);
    on_destroy = std::move(e.on_destroy);
    return *this;
}

void OpenEntry::dispose() {
    if (entry) {
        entry->open_count--;
        entry->close(state);
        if (on_destroy)
            on_destroy(entry);
    }
}
} /* namespace filesystem */
