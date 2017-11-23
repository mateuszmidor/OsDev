/**
 *   @file: Vector.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_VECTOR_H_
#define USER_USTD_SRC_VECTOR_H_

#include <vector>
#include "Allocator.h"

namespace ustd {

template <class T>
using vector = std::vector<T, Allocator<T>>;

} /* namespace ustd */

#endif /* USER_USTD_SRC_VECTOR_H_ */
