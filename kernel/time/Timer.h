/**
 *   @file: Timer.h
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_TIME_TIMER_H_
#define KERNEL_TIME_TIMER_H_

#include <functional>
#include "types.h"

namespace ktime {

using OnTimerExpire = std::function<void()>;

class Timer {
public:
    Timer(s64 remaining_ticks, const OnTimerExpire& on_expire) : remaining_ticks(remaining_ticks), reload_ticks(0), on_expire(on_expire) {}
    Timer(s64 remaining_ticks, u32 reload_ticks, const OnTimerExpire& on_expire) : remaining_ticks(remaining_ticks), reload_ticks(reload_ticks), on_expire(on_expire) {}

    OnTimerExpire   on_expire;
    s64             remaining_ticks;        // how many clock ticks left to expire
    u32             reload_ticks;           // value to reset the remaining_ticks to after timer expires
    u64             timer_id        {0};    // set by TimeManager when adding timer
};

} /* namespace time */

#endif /* KERNEL_TIME_TIMER_H_ */
