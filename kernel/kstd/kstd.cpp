/**
 *   @file: kstd.cpp
 *
 *   @date: Jun 9, 2017
 * @author: mateusz
 */

#include "kstd.h"

extern "C" void __cxa_pure_virtual() {
    __builtin_unreachable();
}

namespace std {

void __throw_length_error(char const* s) {
    __builtin_unreachable();
}

void __throw_logic_error(char const* s) {
    __builtin_unreachable();
}

void __throw_bad_alloc() {
    __builtin_unreachable();
}

void __throw_out_of_range_fmt(char const* s, ...) {
    __builtin_unreachable();
}

void __throw_bad_function_call() {
    __builtin_unreachable();
}

}
