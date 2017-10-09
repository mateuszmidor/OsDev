/**
 *   @file: FrameAllocator.h
 *
 *   @date: Oct 9, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MEMORY_FRAMEALLOCATOR_H_
#define SRC_MEMORY_FRAMEALLOCATOR_H_

#include  <stddef.h>
#include "types.h"

namespace memory {

class FrameAllocator {
public:
    static void init(size_t first_available_memory_byte, size_t last_available_memory_byte);
    static size_t alloc_frame();
    static void free_frame(size_t physical_address);
    static size_t get_used_frames_count();
    static size_t get_total_frames_count();
    static size_t get_frame_size();

private:
    static const    u32 FRAME_SIZE = 1024*1024 * 2; // 2MB pages
    static bool     frame_bitmap[]; // each bit maps 1 page
};

} /* namespace memory */

#endif /* SRC_MEMORY_FRAMEALLOCATOR_H_ */
