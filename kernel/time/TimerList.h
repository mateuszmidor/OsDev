/**
 *   @file: TimerList.h
 *
 *   @date: Nov 14, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_TIME_TIMERLIST_H_
#define KERNEL_TIME_TIMERLIST_H_

#include "List.h"

namespace ktime {

// forward declaration
class Timer;

/**
 * @brief   This class is a list of system timers
 */
class TimerList: public kstd::List<Timer*> {
public:
    void push_sorted_ascending_by_expire_time(Timer* t);

};

} /* namespace time */

#endif /* KERNEL_TIME_TIMERLIST_H_ */
