/**
 *   @file: String.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "String.h"

namespace ustd {

/**
 * @brief   Implementation of <cstring> "strlen" function
 */
extern "C" size_t strlen ( const char * str ) {
    size_t result = 0;
    while (*str) {
        str++;
        result++;
    }

    return result;
}

} /* namespace ustd */
