/**
 *   @file: WyoosAllocationPolicy.cpp
 *
 *   @date: Nov 16, 2017
 * @author: Mateusz Midor
 */

#include "Assert.h"
#include "WyoosAllocationPolicy.h"

using namespace utils;
namespace memory {

/**
 * Constructor.
 * @param   first_byte  The first byte of memory available for allocation
 * @param   last_byte   The last byte of memory available for allocation
 */
WyoosAllocationPolicy::WyoosAllocationPolicy(size_t first_byte, size_t last_byte):
        AllocationPolicy(first_byte, last_byte) {

    phobos_assert(last_byte - first_byte >= sizeof(MemoryChunk), "Not enough memory given to init WyoosAllocationPolicy.");

    // make entire available addresspace a big MemoryChunk
    m_head =  MemoryChunk::create(first_byte);
    m_head->size = last_byte - first_byte - sizeof(MemoryChunk);
}

/**
 * @brief   Find and alloc MemoryChunk that can accomodate "size" bytes
 */
void* WyoosAllocationPolicy::alloc_bytes(size_t size) {
    MemoryChunk* curr = m_head;
    while (curr) {
        if (!curr->is_allocated() && (curr->size >= size)) { // free chunk that is big enough found
            split_chunk_if_worthwhile(curr, size);
            curr->alloc();
            return curr->get_data();
        }

        curr = curr->next;
    }

    // no chunk that could accommodate "size" bytes found
    return nullptr;
}

/**
 * @brief   Split the chunk if the resulting new chunk could accommodate at least 1 byte of data
 * @note
 * +-----------------------+      +------------------------+
 * |HDR|        DATA       |  ->  |HDR|  DATA   |HDR| DATA |
 * +-----------------------+      + -----------------------+
 */
void WyoosAllocationPolicy::split_chunk_if_worthwhile(MemoryChunk* c, size_t chop_size) {
    const size_t HDR_SIZE = sizeof(MemoryChunk);
    const size_t MIN_SIZE = HDR_SIZE + 1;    // so we would be able to alloc at least 1 byte
    const ssize_t REMAINING_SIZE = c->size - chop_size;

    // dont split the chunk as the resulting second chunk would be too small
    if (REMAINING_SIZE < MIN_SIZE)
        return;

    // lets split the chunk into 2 chunks
    size_t next_addr = (size_t)c->get_data() + chop_size;
    MemoryChunk* next_chunk = MemoryChunk::create(next_addr);
    next_chunk->size = REMAINING_SIZE - HDR_SIZE;
    next_chunk->next = c->next;
    if (next_chunk->next)
        next_chunk->next->prev = next_chunk;
    next_chunk->prev = c;

    c->size = chop_size;
    c->next = next_chunk;
}

/**
 * @brief   Release and possibly merge chunk of memory, if valid
 */
void WyoosAllocationPolicy::free_bytes(void* address) {
    MemoryChunk* c = MemoryChunk::from_data(address);
    if (!c->is_valid()) {
//       "Invalid address given to free\n";
        return;
    }

    if (!c->is_allocated()) {
//        "Given memory already frieed\n";
        return;
    }

    c->dealloc();

    // following merge must be done in this specific order since merge (c->prev, c) can result in deleting "c" MemoryChunk
    merge_chunks_if_possible(c, c->next);
    merge_chunks_if_possible(c->prev, c);
}

/**
 * @brief   Merge two unallocated MemoryChunks into one big chunk
 */
void WyoosAllocationPolicy::merge_chunks_if_possible(MemoryChunk* c1, MemoryChunk* c2) {
    if (!c1 || c1->is_allocated() || !c2 || c2->is_allocated())
        return;

    const size_t HDR_SIZE = sizeof(MemoryChunk);
    size_t NEW_SIZE = c1->size + HDR_SIZE + c2->size;

    c1->size = NEW_SIZE;
    c1->next = c2->next;
    if (c1->next)
        c1->next->prev = c1;

    MemoryChunk::destroy(c2);
}

/**
 * @brief   Calc total free memory that is contained in unallocated chunks
 */
size_t WyoosAllocationPolicy::free_memory_in_bytes() {
    size_t total_free = 0;
    MemoryChunk* curr = m_head;
    while (curr) {
        if (!curr->is_allocated())
            total_free += curr->size;

        curr = curr->next;
    }

    return total_free;
}

/**
 * @brief   Get total memory available for allocation
 */
size_t WyoosAllocationPolicy::total_memory_in_bytes() {
    return available_memory_last_byte - available_memory_first_byte;
}

} /* namespace memory */
