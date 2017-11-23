/**
 *   @file: ustd.cpp
 *
 *   @date: Oct 28, 2017
 * @author: Mateusz Midor
 */

#include "ustd.h"


extern "C" void __cxa_pure_virtual() {
//    klog.format("pure virtual function called\n");
}

void std::__throw_length_error(char const* s) {
//    klog.format("__throw_length_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_logic_error(char const* s) {
//    klog.format("__throw_logic_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_alloc() {
//    klog.format("__throw_bad_alloc\n");
    __builtin_unreachable();
}

void std::__throw_out_of_range_fmt(char const* s, ...) {
//    klog.format("__throw_out_of_range_fmt: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_function_call() {
//    klog.format("__throw_bad_function_call\n");
    __builtin_unreachable();
}
