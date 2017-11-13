/**
 *   @file: TimeManager.cpp
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#include "TimeManager.h"

namespace time {

TimeManager TimeManager::_instance;

TimeManager& TimeManager::instance() {
    return _instance;
}

void TimeManager::tick() {
    tick_count++;
}

u64 TimeManager::get_ticks() const {
    return tick_count;
}

void TimeManager::set_hz(u64 tick_frequency_in_hz) {
    tick_frequency = tick_frequency_in_hz;
}

u64 TimeManager::get_hz() const {
    return tick_frequency;
}

} /* namespace time */
