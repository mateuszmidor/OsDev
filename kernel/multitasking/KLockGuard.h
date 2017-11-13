/**
 *   @file: KLockGuard.h
 *
 *   @date: Nov 13, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MULTITASKING_KLOCKGUARD_H_
#define KERNEL_MULTITASKING_KLOCKGUARD_H_

#include "types.h"

namespace multitasking {

/**
 * @class   This class is a guard lock that disables interrupts for its lifetime; for protecting kernel structures access
 */
class KLockGuard {
public:
    KLockGuard();
    virtual ~KLockGuard();

private:
    u64 rflags = 0;
};

} /* namespace multitasking */

#endif /* KERNEL_MULTITASKING_KLOCKGUARD_H_ */
