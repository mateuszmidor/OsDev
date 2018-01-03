/**
 *   @file: Conversions.h
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_CONVERSIONS_H_
#define USER_USTD_SRC_CONVERSIONS_H_

#include "types.h"
#include "String.h"

namespace ustd {
namespace conversions {

string int_to_string(s64 num, u8 base = 10);
string double_to_string(double num);
s64 string_to_int(const string& str);
double string_to_double(const string& str);

} /* namespace conversions */

} /* namespace ustd */

#endif /* USER_USTD_SRC_CONVERSIONS_H_ */
