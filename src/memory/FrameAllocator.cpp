/**
 *   @file: FrameAllocator.cpp
 *
 *   @date: Oct 9, 2017
 * @author: Mateusz Midor
 */

#include "FrameAllocator.h"

namespace memory {

bool   FrameAllocator::frame_bitmap[64]; // map 64 * 2MB pages = 128MB of physical memory

void FrameAllocator::init(size_t first_available_memory_byte, size_t last_available_memory_byte) {
    u32 first_index = first_available_memory_byte / FRAME_SIZE + 1;
    u32 last_index = last_available_memory_byte / FRAME_SIZE;

    for (u32 i = 0; i < first_index; i++)
        frame_bitmap[i] = true;

    for (u32 i = last_index + 1; i <  get_total_frames_count(); i++)
        frame_bitmap[i] = true;
}

/**
 * @brief   Find first free frame and return it's physical address
 */
size_t FrameAllocator::alloc_frame() {
    for (u32 i = 0; i < get_total_frames_count(); i++)
        if (!frame_bitmap[i]) {
            frame_bitmap[i] = true;
            return i * FRAME_SIZE;
        }

    return 0; // no free page available
}

void FrameAllocator::free_frame(size_t physical_address) {
    u32 index = physical_address / FRAME_SIZE;
    if (index < get_total_frames_count())
        frame_bitmap[index] = false;
}

size_t FrameAllocator::get_used_frames_count() {
    size_t result = 0;

    for (u32 i = 0; i < get_total_frames_count(); i++)
        if (frame_bitmap[i])
            result++;

    return result;
}

size_t FrameAllocator::get_total_frames_count() {
    return sizeof(frame_bitmap) / sizeof(frame_bitmap[0]);
}

size_t FrameAllocator::get_frame_size() {
    return FRAME_SIZE;
}

} /* namespace memory */
