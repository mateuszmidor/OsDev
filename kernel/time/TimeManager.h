/**
 *   @file: TimeManager.h
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_TIME_TIMEMANAGER_H_
#define KERNEL_TIME_TIMEMANAGER_H_

#include "types.h"
//#include "TimerList.h"

namespace time {

/**
 * @brief   This class provides time related functionality, like current tick, scheduling events(timers)
 */
class TimeManager {
public:
    static TimeManager& instance();
    void tick();
    u64 get_ticks() const;
    void set_hz(u64 tick_frequency_in_hz);
    u64 get_hz() const;

private:
    TimeManager() {}

    static TimeManager _instance;
    u64 tick_count      = 0;
    u64 tick_frequency  = 1;
};

} /* namespace time */

#endif /* KERNEL_TIME_TIMEMANAGER_H_ */
