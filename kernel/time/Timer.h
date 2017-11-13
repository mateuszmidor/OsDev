/**
 *   @file: Timer.h
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_TIME_TIMER_H_
#define KERNEL_TIME_TIMER_H_

#include "types.h"

namespace time {

class Timer {
private:
    u64 ticks_left;     // how many clock ticks left to expire
};

} /* namespace time */

#endif /* KERNEL_TIME_TIMER_H_ */
