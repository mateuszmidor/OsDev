/**
 *   @file: Mutex.cpp
 *
 *   @date: Dec 1, 2017
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "Mutex.h"

namespace cstd {
namespace ustd {

/**
 * @brief   Busy-waiting mutex lock for now
 */
void Mutex::lock() {
    bool expected_value = false;
    while (!is_locked.compare_exchange_strong(expected_value, true)) {
        expected_value = false; // if we got here, the expected_value got updated to true so need to reset it
        syscalls::msleep(0); // yield
    }
}

void Mutex::unlock() {
    is_locked.store(false);
}

} /* namespace ustd */
} /* namespace cstd */
