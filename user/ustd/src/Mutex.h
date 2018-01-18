/**
 *   @file: Mutex.h
 *
 *   @date: Dec 1, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_MUTEX_H_
#define USER_USTD_SRC_MUTEX_H_

#include <atomic>

namespace cstd {
namespace ustd {

class Mutex {
public:
    void lock();
    void unlock();

private:
    std::atomic<bool> is_locked { false };
};

} /* namespace ustd */
} /* namespace cstd */

#endif /* USER_USTD_SRC_MUTEX_H_ */
