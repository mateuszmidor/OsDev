/**
 *   @file: Vector.h
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_KSTD_VECTOR_H_
#define KERNEL_KSTD_VECTOR_H_

#include <vector>
#include "Allocator.h"

namespace kstd {

template <class T>
using vector = std::vector<T, Allocator<T>>;

} /* namespace kstd */

#endif /* KERNEL_KSTD_VECTOR_H_ */
