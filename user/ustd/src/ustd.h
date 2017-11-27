/**
 *   @file: ustd.h
 *
 *   @date: Oct 20, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_USTD_H_
#define USER_USTD_SRC_USTD_H_

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


}; // namespace ustd

#endif /* USER_USTD_SRC_USTD_H_ */
