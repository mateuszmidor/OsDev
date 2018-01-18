/**
 *   @file: cstd.h
 *
 *   @date: Jan 16, 2018
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_CSTD_H_
#define MIDDLESPACE_CSTD_CSTD_H_

#include <type_traits> // common_type
#include "types.h"

/**
 * Memory
 */
extern "C" void * memcpy( void * destination, const void * source, size_t num );
extern "C" void * memmove( void * destination, const void * source, size_t num );
extern "C" int memcmp ( const void * ptr1, const void * ptr2, size_t num );
extern "C" void* memset( void* dest, int ch, size_t count );
extern "C" const void* memchr(const void* ptr, int ch, size_t count);


/**
 * Utils
 */
template <class T1, class T2>
typename std::common_type<T1, T2>::type min(const T1& a, const T2& b) {
    return a < b ? a : b;
}

template <class T1, class T2>
typename std::common_type<T1, T2>::type max(const T1& a, const T2& b) {
    return a > b ? a : b;
}


#endif /* MIDDLESPACE_CSTD_CSTD_H_ */
