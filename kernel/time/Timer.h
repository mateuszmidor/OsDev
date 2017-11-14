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
    Timer(s64 ticks_left, const OnTimerExpire& on_expire) : ticks_left(ticks_left), on_expire(on_expire) {}

    OnTimerExpire   on_expire;
    s64             ticks_left;     // how many clock ticks left to expire
};

} /* namespace time */

#endif /* KERNEL_TIME_TIMER_H_ */
