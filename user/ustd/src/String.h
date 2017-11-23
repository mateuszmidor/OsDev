/**
 *   @file: String.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_STRING_H_
#define USER_USTD_SRC_STRING_H_

#include <string>
#include "types.h"
#include "Allocator.h"

namespace ustd {

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

} /* namespace ustd */

#endif /* USER_USTD_SRC_STRING_H_ */
