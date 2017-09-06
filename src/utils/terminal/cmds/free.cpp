/*
 * free.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: mateusz
 */

#include "free.h"
#include "MemoryManager.h"

using namespace memory;
namespace cmds {

void free::run() {
    MemoryManager& mm = MemoryManager::instance();
    size_t free_memory = mm.get_total_memory_in_bytes() - mm.get_free_memory_in_bytes();
    size_t total_memory = mm.get_total_memory_in_bytes();
    env->printer->format("Used memory so far: % KB, total available: % MB\n", free_memory / 1024, total_memory / 1024 / 1024);
}
} /* namespace cmds */
