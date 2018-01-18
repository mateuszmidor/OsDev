/**
 *   @file: String.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_SRC_STRING_H_
#define MIDDLESPACE_CSTD_SRC_STRING_H_

#include <string>
#include "Allocator.h"

namespace cstd {

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

} /* namespace cstd */

extern "C" size_t strlen ( const char * str );

#endif /* MIDDLESPACE_CSTD_SRC_STRING_H_ */
