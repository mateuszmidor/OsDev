/**
 *   @file: FrameAllocator.h
 *
 *   @date: Oct 9, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_FRAMEALLOCATOR_H_
#define SRC_MEMORY_FRAMEALLOCATOR_H_

#include "types.h"
#include "PageTables.h"

namespace memory {

class FrameAllocator {
public:
    static void init(size_t first_available_memory_byte, size_t last_available_memory_byte);
    static ssize_t alloc_frame();
    static void free_frame(size_t physical_address);
    static ssize_t alloc_consecutive_frames(size_t num_bytes);
    static void free_consecutive_frames(size_t physical_address, size_t num_bytes);
    static size_t get_used_frames_count();
    static size_t get_total_frames_count();
    static constexpr size_t get_frame_size() { return PageTables::get_page_size(); };

private:
    static ssize_t find_unused_starting_at(size_t start_index);
    static size_t check_consecutive_unused_count_at(size_t start_index, size_t required_frames_count);

    static size_t   total_frames;
    static bool     frames_allocated[]; // each entry maps 1 page
};

} /* namespace memory */

#endif /* SRC_MEMORY_FRAMEALLOCATOR_H_ */
