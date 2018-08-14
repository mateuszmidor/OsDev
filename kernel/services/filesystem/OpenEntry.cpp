/**
 *   @file: OpenEntry.cpp
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntry.h"

using namespace middlespace;

namespace filesystem {

OpenEntry::OpenEntry(VfsCachedEntryPtr e, EntryState* s, const OnDestroy& on_destroy) : entry(e), state(s), on_destroy(on_destroy) {
    if (entry)
        entry->open_count++;
}

OpenEntry::~OpenEntry() {
    if (entry) {
        entry->open_count--;
        entry->close(state);
        if (on_destroy)
            on_destroy(entry);
    }
}

} /* namespace filesystem */
