/**
 *   @file: TimeManager.cpp
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#include "TimeManager.h"
#include "KLockGuard.h"

using namespace multitasking;
namespace ktime {

TimeManager TimeManager::_instance;

TimeManager& TimeManager::instance() {
    return _instance;
}

/**
 * @brief   Clock tick function; makes the time pass
 * @note    Execution context: Interrupt only (Programmable Interval Timer)
 */
// TODO: improve so no need to update all timers every tick
void TimeManager::tick() {
    total_tick_count++;

    // return if there is no timers to process
    if (timers.count() == 0)
        return;

    // time passes
    for (Timer* t : timers)
        t->ticks_left--;

    // does the soonest timer expire?
    if (timers.front()->ticks_left > 0)
        return;

    // soonest timer expires
    Timer* t = timers.pop_front();

    // run expire and get rid of the timer
    t->on_expire();
    delete t;
}

u64 TimeManager::get_ticks() const {
    return total_tick_count;
}

void TimeManager::set_hz(u64 tick_frequency_in_hz) {
    tick_frequency = tick_frequency_in_hz;
}

u64 TimeManager::get_hz() const {
    return tick_frequency;
}

/**
 * @brief   Emplace a new timer
 * @param   millis After how many milliseconds to expire
 * @param   on_expire What to do on expire
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
void TimeManager::emplace(u32 millis, const OnTimerExpire& on_expire) {
    KLockGuard lock;    // prevent reschedule

    u32 ticks = get_hz() * millis / 1000;
    Timer* t = new Timer(ticks, on_expire);
    timers.push_sorted_ascending_by_expire_time(t);
}

} /* namespace time */
