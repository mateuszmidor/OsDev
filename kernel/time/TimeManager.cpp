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
        t->remaining_ticks--;

    // does the soonest timer expire?
    if (timers.front()->remaining_ticks > 0)
        return;

    // soonest timer expires
    Timer* t = timers.pop_front();

    // run expire and reload/remove timer
    t->on_expire();
    if (t->reload_ticks) {
        t->remaining_ticks = t->reload_ticks;
        timers.push_sorted_ascending_by_expire_time(t);
    } else
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
 * @brief   Emplace a new one-shoot timer
 * @param   expire_millis After how many milliseconds to expire
 * @param   on_expire What to do on expire
 * @return  Newly added timer id
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
u64 TimeManager::emplace(u32 expire_millis, const OnTimerExpire& on_expire) {
    return emplace(expire_millis, 0, on_expire);
}

/**
 * @brief   Emplace a new reloadable timer
 * @param   expire_millis After how many milliseconds to expire
 * @param   reload_millis Value to reset the timer to after it expires, 0 means no reload
 * @param   on_expire What to do on expire
 * @return  Newly added timer id
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
u64 TimeManager::emplace(u32 expire_millis, u32 reload_millis, const OnTimerExpire& on_expire) {
    KLockGuard lock;    // prevent reschedule

    u32 remaining_ticks = get_hz() * expire_millis / 1000;
    u32 reload_ticks = get_hz() * reload_millis / 1000;
    Timer* t = new Timer(remaining_ticks, reload_ticks, on_expire);
    t->timer_id = next_timer_id;
    timers.push_sorted_ascending_by_expire_time(t);
    next_timer_id++;
    return t->timer_id;
}

/**
 * @brief   Cancel timer of given id
 * @note    Execution context: Task/Interrupt; be careful with possible reschedule during execution of this method
 */
void TimeManager::cancel(u64 timer_id) {
    KLockGuard lock;    // prevent reschedule

    if (Timer* t = timers.get_by_tid(timer_id))
        timers.remove(timers.find(t));
}

} /* namespace time */
