/**
 *   @file: WyoosAllocationPolicy.h
 *
 *   @date: Nov 16, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MEMORY_WYOOSALLOCATIONPOLICY_H_
#define KERNEL_MEMORY_WYOOSALLOCATIONPOLICY_H_

#include <new>
#include "AllocationPolicy.h"

namespace cstd {

/**
 * @brief   This class represents an element of doubly-linked list of memory chunks
 * @note    Memory chunk is organized as this:
 *          +-----------------------+
 *          |HDR|       DATA        |
 *          +-----------------------+
 */
struct MemoryChunk {
    static const size_t MAGIC_UNALLOCATED   = 1234567890; // even number means its not allocated
    size_t                  magic_allocated = MAGIC_UNALLOCATED;
    size_t                  size            = 0;
    MemoryChunk*            prev            = nullptr;
    MemoryChunk*            next            = nullptr;

    static MemoryChunk* create(size_t addr)     { return new ((MemoryChunk*)addr) MemoryChunk; }
    static void destroy(MemoryChunk* c)         { c->prev = nullptr; c->next = nullptr; c->size = 0; c->magic_allocated = 0; }
    static MemoryChunk* from_data(void* addr)   { return (MemoryChunk*) ((size_t)addr - sizeof(MemoryChunk)); }
    void* get_data()                            { return (void*) ((size_t)this + sizeof(MemoryChunk)); }
    void alloc()                                { magic_allocated |= 1; }
    void dealloc()                              { magic_allocated &= ~1; }
    bool is_valid() const                       { return (magic_allocated & ~1) == MAGIC_UNALLOCATED; }
    bool is_allocated() const                   { return magic_allocated & 1; };
};

/**
 * @brief   This class is an allocation policy based on "Write your own Operating System" series.
 *          We start with whole memory seen as a big memory chunk that is later split on malloc and merged on free.
 *          For simplicity, just one doubly-linked list of allocated and free memory chunks is used.
 * @see     https://www.youtube.com/watch?v=BeSpPd3C3J8
 */
class WyoosAllocationPolicy: public AllocationPolicy {
public:
    WyoosAllocationPolicy(size_t first_byte, size_t last_byte);

    void* alloc_bytes(size_t size) override;
    void free_bytes(void* address) override;
    size_t free_memory_in_bytes() override;
    size_t total_memory_in_bytes() override;

private:
    void split_chunk_if_worthwhile(MemoryChunk* c, size_t chop_size);
    void merge_chunks_if_possible(MemoryChunk* c1, MemoryChunk* c2);

    MemoryChunk* m_head;
};

} /* namespace cstd */

#endif /* KERNEL_MEMORY_WYOOSALLOCATIONPOLICY_H_ */
