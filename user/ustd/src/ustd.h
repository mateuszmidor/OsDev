/**
 *   @file: ustd.h
 *
 *   @date: Oct 20, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_USTD_H_
#define USER_USTD_SRC_USTD_H_

namespace std {

void __throw_length_error(char const* s);
void __throw_logic_error(char const* s);
void __throw_bad_alloc();
void __throw_out_of_range_fmt(char const* s, ...);
void __throw_bad_function_call();

};

#endif /* USER_USTD_SRC_USTD_H_ */
