/**
 *   @file: kstd.h
 *
 *   @date: Jun 8, 2017
 * @author: Mateusz Midor
 *   @note: This file provides minimum of stdlib functionality
 */

#ifndef SRC_KSTD_H_
#define SRC_KSTD_H_

#include <utility>
#include <cstring>


namespace kstd {

template <class T1, class T2>
typename std::common_type<T1, T2>::type min(const T1& a, const T2& b) {
    return a < b ? a : b;
}

template <class T1, class T2>
typename std::common_type<T1, T2>::type max(const T1& a, const T2& b) {
    return a > b ? a : b;
}


}; // namespace kstd

#endif /* SRC_KSTD_H_ */
