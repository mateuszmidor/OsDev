/**
 *   @file: Timer.cpp
 *
 *   @date: Jan 19, 2018
 * @author: Mateusz Midor
 */

#include "syscalls.h"
#include "Timer.h"

namespace cstd {
namespace ustd {

Timer::Timer() {
    timespec ts;
    syscalls::clock_gettime(CLOCK_MONOTONIC, &ts);
    last_sec = ts.tv_sec;
    last_nsec = ts.tv_nsec;
}

double Timer::get_delta_seconds() {
    constexpr double NSEC = 1000*1000*1000;
    timespec ts;
    syscalls::clock_gettime(CLOCK_MONOTONIC, &ts);
    double dt = (ts.tv_sec - last_sec) + (ts.tv_nsec - last_nsec) / NSEC;
    last_sec = ts.tv_sec;
    last_nsec = ts.tv_nsec;
    return dt;
}

} /* namespace ustd */
} /* namespace cstd */
