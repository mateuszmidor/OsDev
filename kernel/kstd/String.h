/**
 *   @file: String.h
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_KSTD_STRING_H_
#define KERNEL_KSTD_STRING_H_

#include <string>
#include "Allocator.h"

namespace kstd {

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;;

} /* namespace kstd */

#endif /* KERNEL_KSTD_STRING_H_ */
