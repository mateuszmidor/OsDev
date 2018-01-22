/**
 *   @file: Timer.h
 *
 *   @date: Jan 19, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_TIMER_H_
#define USER_USTD_SRC_TIMER_H_

#include "types.h"

namespace cstd {
namespace ustd {

class Timer {
public:
    Timer();
    double get_delta_seconds();

private:
    s64    last_sec;    // these must be signed as clock_gettime may return eg. 750ms as 1 sec -250msec
    s64    last_nsec;
};

} /* namespace ustd */
} /* namespace cstd */

#endif /* USER_USTD_SRC_TIMER_H_ */
