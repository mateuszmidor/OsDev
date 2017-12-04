/**
 *   @file: ScopeGuard.h
 *
 *   @date: Dec 4, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_SCOPEGUARD_H_
#define USER_USTD_SRC_SCOPEGUARD_H_

#include "Mutex.h"

namespace ustd {

/**
 * @brief   This class ensures thread-exclusive access within its scope of life
 */
class ScopeGuard {
public:
    ScopeGuard() { mtx.lock(); }
    virtual ~ScopeGuard() { mtx.unlock(); }

private:
    Mutex mtx;
};

} /* namespace ustd */

#endif /* USER_USTD_SRC_SCOPEGUARD_H_ */
