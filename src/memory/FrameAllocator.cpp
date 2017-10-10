/**
 *   @file: FrameAllocator.cpp
 *
 *   @date: Oct 9, 2017
 * @author: Mateusz Midor
 */

#include "FrameAllocator.h"

namespace memory {

bool   FrameAllocator::frame_allocated[64]; // map 64 * 2MB pages = 128MB of physical memory

void FrameAllocator::init(size_t first_available_memory_byte, size_t last_available_memory_byte) {
    const size_t first_index = first_available_memory_byte / FRAME_SIZE + 1;
    const size_t last_index = last_available_memory_byte / FRAME_SIZE;

    for (size_t i = 0; i < first_index; i++)
        frame_allocated[i] = true;

    for (size_t i = last_index + 1; i < get_total_frames_count(); i++)
        frame_allocated[i] = true;
}

/**
 * @brief   Find first free frame and allocate it
 * @return  -1 on failure, frame physical address on success
 */
ssize_t FrameAllocator::alloc_frame() {
    const ssize_t index = find_unused_starting_at(0);

    if (index == -1)
        return -1; // no free page available

    frame_allocated[index] = true;
    return index * FRAME_SIZE;
}

/**
 * @brief   Find frame that starts at "physical_address" and deallocate it
 */
void FrameAllocator::free_frame(size_t physical_address) {
    const size_t index = physical_address / FRAME_SIZE;
    if (index < get_total_frames_count())
        frame_allocated[index] = false;
}

/**
 * @brief   Find and allocate a chain of consecutive frames that is capable to hold at least "num_bytes"
 * @return  -1 on failure, first frame physical address on success
 */
ssize_t FrameAllocator::alloc_consecutive_frames(size_t num_bytes) {
    const size_t num_needed_frames = num_bytes / FRAME_SIZE + 1;

    ssize_t index = 0;
    size_t consecutive_unused_count = 0;
    while ((index = find_unused_starting_at(index)) > -1) {
        consecutive_unused_count = check_consecutive_unused_count_at(index, num_needed_frames);
        if (consecutive_unused_count == num_needed_frames)
            break; // found
        else
            index += consecutive_unused_count; // continue search
    }

    // no long enough chain of frames found
    if (consecutive_unused_count < num_needed_frames)
        return -1;

    // mark frames as allocated
    for (size_t i = index; i < index + num_needed_frames; i++)
        frame_allocated[i] = true;

    return index  * FRAME_SIZE;
}

/**
 * @brief   Find first free frame index starting at "start_index"
 * @return  -1 on failure, frame index on success
 */
ssize_t FrameAllocator::find_unused_starting_at(size_t start_index) {
    for (size_t i = start_index; i < get_total_frames_count(); i++)
        if (!frame_allocated[i])
            return i;

    return -1;  // not found
}

/**
 * @brief   Check how many consecutive free frames start at "start_index", check for at most "required_frames_count"
 */
size_t FrameAllocator::check_consecutive_unused_count_at(size_t start_index, size_t required_frames_count) {
    size_t result = 0;
    for (size_t i = start_index; i < get_total_frames_count(); i++) {
        if (result == required_frames_count)
            break;

        if (frame_allocated[i])
           break; // break if no more consecutive free frames

        result++;
    }

    return result;
}

/**
 * @brief   Find consecutive frame chain that starts at "physical_address" and deallocate all its frames
 */
void FrameAllocator::free_consecutive_frames(size_t physical_address, size_t num_bytes) {
    const size_t index = physical_address / FRAME_SIZE;
    const size_t num_frames = num_bytes / FRAME_SIZE + 1;

    if (index < get_total_frames_count())
        for (size_t i = index; i < index + num_frames; i++)
            frame_allocated[index] = false;
}

/**
 * @brief   How many frames are currently allocated?
 */
size_t FrameAllocator::get_used_frames_count() {
    size_t result = 0;

    for (u32 i = 0; i < get_total_frames_count(); i++)
        if (frame_allocated[i])
            result++;

    return result;
}

/**
 * @brief   What is the total amount of available frames?
 */
size_t FrameAllocator::get_total_frames_count() {
    return sizeof(frame_allocated) / sizeof(frame_allocated[0]);
}

/**
 * @brief   What is the frame size in bytes?
 */
size_t FrameAllocator::get_frame_size() {
    return FRAME_SIZE;
}

} /* namespace memory */
