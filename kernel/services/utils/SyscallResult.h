/**
 *   @file: SyscallResult.h
 *
 *   @date: Feb 14, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_UTILS_SYSCALLRESULT_H_
#define KERNEL_SERVICES_UTILS_SYSCALLRESULT_H_

#include <utility> 	// std::move
#include "ErrorCode.h"

namespace utils {

/**
 * @brief   This class carries system call result(if success) and error code
 */
template <class T>
class SyscallResult {
public:
    SyscallResult(const T& value) : value(value), ec(middlespace::ErrorCode::EC_OK) {}
    SyscallResult(T&& value) : value(std::move(value)), ec(middlespace::ErrorCode::EC_OK) {}
    SyscallResult(middlespace::ErrorCode error) : value(T{}), ec(error) {}
    explicit operator bool() const { return ec == middlespace::ErrorCode::EC_OK; }
    bool operator!() const { return ec != middlespace::ErrorCode::EC_OK; }

    T                       value;
    middlespace::ErrorCode  ec;
};

/**
 * @brief   SyscallResult specialization that only carries ErrorCode and no data
 */
template <>
class SyscallResult<void> {
public:
    SyscallResult(middlespace::ErrorCode error) : ec(error) {}
    explicit operator bool() const { return ec == middlespace::ErrorCode::EC_OK; }
    bool operator!() const { return ec != middlespace::ErrorCode::EC_OK; }

    middlespace::ErrorCode  ec;
};

} /* namespace utils */

#endif /* KERNEL_SERVICES_UTILS_SYSCALLRESULT_H_ */
