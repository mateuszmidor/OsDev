/**
 *   @file: KLockGuard.cpp
 *
 *   @date: Nov 13, 2017
 * @author: Mateusz Midor
 */

#include "KLockGuard.h"

namespace multitasking {

KLockGuard::KLockGuard() {
    asm volatile("pushfq; pop %0; cli;" : "=g" (rflags));

}

KLockGuard::~KLockGuard() {
    asm volatile("push %0; popfq; " :: "g" (rflags));
}

} /* namespace multitasking */
