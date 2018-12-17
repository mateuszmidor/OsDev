/**
 *   @file: Assert.h
 *
 *   @date: Nov 16, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_UTILS_ASSERT_H_
#define KERNEL_UTILS_ASSERT_H_

#include "types.h"

namespace utils {

void phobos_assert(bool condition, const char* msg);
void phobos_halt();

} /* namespace utils */

#endif /* KERNEL_UTILS_ASSERT_H_ */
