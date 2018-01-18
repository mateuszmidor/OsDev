/**
 *   @file: Math.cpp
 *
 *   @date: Dec 22, 2017
 * @author: Mateusz Midor
 */

#include "Math.h"

namespace cstd {
namespace ustd {
namespace math {

double sqrt(double x) {
    double result;
    asm volatile("fsqrt" : "=t"(result) : "0"(x));
    return result;
}

double sin(double radians) {
    double result;
    asm volatile("fsin" : "=t"(result) : "0"(radians));
    return result;
}

double cos(double radians) {
    double result;
    asm volatile("fcos" : "=t"(result) : "0"(radians));
    return result;
}

/**
 * @brief   2^x - 1;
 * @param   x Exponent. Must fall between -1..1, otherwise result undefined
 */
double F2XM1(double x) {
    double result {0.0};
    asm volatile("f2xm1" : "=t"(result) : "0"(x));
    return result;
}

/**
 * @brief   y ∗ Log2 (x);
 * @param   x Base. Must be > 0
 * @param   y Exponent. Any number
 */
double FYL2X(double x, double y) {
    double result {0.0};
    asm volatile(
            "fyl2x;"
            : "=t"(result)
            : "0"(x), "u"(y)
            : "st(1)"
    );
    return result;
}

/**
 * @brief   x^y where y is an integer and x can be negative
 */
double pow_int(double x, long long y) {
    if (y < 0) {
        x = 1.0 / x;
        y = -y;
    }

    double result = x;
    for (unsigned i = 1; i < y; i++) {
        result *= x;
    }
    return result;
}

/**
 * @brief   x^y
 * @param   x Base
 * @param   y Exponent.
 * @note    x^y =2^[y*log2(x)] ; so there is inner part a=y*log2(x) and outer part 2^a
 * @see     http://www.website.masmforum.com/tutorials/fptute/fpuchap11.htm, "F2XM1 (2 to the X power Minus 1)"
 */
double pow(double x, double y) {
    if (x < 0 && y == (long long)y)
        return pow_int(x, y);

    double result {};
    long fpu_flags {};
    long fpu_flags_copy {};
    asm volatile(
            // calc inner part of the equation
            "fyl2x              ;"      // st(0) = y ∗ Log2(x); where x > 0
            "fld %%st(0)        ;"      // st(1) = st(0)

            // set rounding to truncate
            "fstcw  %4          ;"      // save fpu control word to fpu_flags_copy
            "fstcw  %3          ;"
            "mov %3, %%ebx      ;"
            "or $0xC, %%bh      ;"      // 0xC for truncate
            "mov %%ebx, %3      ;"
            "fldcw  %3          ;"

            "frndint            ;"      // st(0) = trunc(st(0))

            // restore control word
            "fldcw  %4 ;"

            // calc outer part of the equation
            "fxch %%st(1) ;"            // swap(st(0), st(1))
            "fsub %%st(1), %%st(0);"    // st(0) = frac(st(0))
            "f2xm1 ;"                   // st(0) = (2 ^ st(0)) - 1
            "fld1 ;"
            "faddp ;"                   // st(0) += 1
            "fscale ;"                  // 2^(a+b) = 2^a * 2 ^b
            "fstp %%st(1)"              // st(1) = st(0); pop st(0)
            : "=t"(result)
            : "0"(x), "u"(y), "m"(fpu_flags), "m"(fpu_flags_copy)
            : "st(1)", "st(2)", "rbx"
    );
    return result;
}

} /* namespace math */
} /* namespace ustd */
} /* namespace cstd */
