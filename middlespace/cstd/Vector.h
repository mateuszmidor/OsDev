/**
 *   @file: Vector.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_SRC_VECTOR_H_
#define MIDDLESPACE_CSTD_SRC_VECTOR_H_

#include <vector>
#include "Allocator.h"

namespace cstd {

template <class T>
using vector = std::vector<T, Allocator<T>>;

} /* namespace cstd */

#endif /* MIDDLESPACE_CSTD_SRC_VECTOR_H_ */
