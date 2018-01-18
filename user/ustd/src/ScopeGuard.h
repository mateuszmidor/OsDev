/**
 *   @file: ScopeGuard.h
 *
 *   @date: Dec 4, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_SCOPEGUARD_H_
#define USER_USTD_SRC_SCOPEGUARD_H_

#include "Mutex.h"

namespace cstd {
namespace ustd {

/**
 * @brief   This class ensures thread-exclusive access within its scope of life
 */
class ScopeGuard {
public:
    ScopeGuard(Mutex& mtx) : mtx(mtx) { mtx.lock(); }
    virtual ~ScopeGuard() { mtx.unlock(); }

private:
    Mutex& mtx;
};

} /* namespace ustd */
} /* namespace cstd */

#endif /* USER_USTD_SRC_SCOPEGUARD_H_ */
