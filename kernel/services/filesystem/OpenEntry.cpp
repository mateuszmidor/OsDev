/**
 *   @file: OpenEntry.cpp
 *
 *   @date: Aug 13, 2018
 * @author: Mateusz Midor
 */

#include "OpenEntry.h"

using namespace middlespace;

namespace filesystem {

OpenEntry::OpenEntry(const OnDestroy& on_destroy, VfsCachedEntryPtr e, EntryState* s) : entry(e), state(s), on_destroy(on_destroy) {
    if (entry)
        entry->open_count++;
}

OpenEntry::~OpenEntry() {
    if (entry && on_destroy) {
        entry->open_count--;
        entry->close(state);
        on_destroy(entry);
    }
}

} /* namespace filesystem */
