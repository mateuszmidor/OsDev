/**
 *   @file: ExclusiveThreadAccess.h
 *
 *   @date: Dec 1, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_MONITOR_H_
#define USER_USTD_SRC_MONITOR_H_

#include <memory>
#include "Mutex.h"

namespace cstd {
namespace ustd {

/**
 * @brief   This class is an exclusive access object to the Monitor held object, it releases the mutex on destroy
 */
template <class T>
class MonitorAccess {
public:
    MonitorAccess(std::shared_ptr<T> obj, Mutex& mtx) : obj(obj), mtx(mtx) { }
    ~MonitorAccess()    { mtx.unlock(); } // unlock the mutex on access object destruction
    T* operator->()     { return obj.get();  }

private:
    std::shared_ptr<T> obj;
    Mutex& mtx;
};

/**
 * @brief    This class ensures thread-exclusive access to the object it holds.
 */
template <class T>
class Monitor {
public:
    void reset(std::shared_ptr<T> obj) { this->obj = obj; }
    MonitorAccess<T> get() { mtx.lock(); return MonitorAccess<T>(obj, mtx); } // this function gives thread-exclusive access to the object

private:
    std::shared_ptr<T> obj;
    Mutex mtx;
};

} /* namespace ustd */
} /* namespace cstd */

#endif /* USER_USTD_SRC_MONITOR_H_ */
