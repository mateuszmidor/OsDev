/**
 *   @file: kstd.h
 *
 *   @date: Jun 8, 2017
 * @author: Mateusz Midor
 *   @note: This file provides minimum of stdlib functionality
 */

#ifndef SRC_KSTD_H_
#define SRC_KSTD_H_



#include "types.h"
#include "Vector.h"
#include "String.h"
#include "StringUtils.h"

extern "C" void * memcpy(void * destination, const void * source, size_t num);
extern "C" void* memset(void* dest, int ch, size_t count);
extern "C" const void* memchr(const void* ptr, int ch, std::size_t count);
//extern "C" void* memchr(void* ptr, int ch, std::size_t count);


namespace kstd {

template <class T>
class Optional : public T {
public:
    Optional(const T& t) : T(t), valid(true) {}
    Optional(T&& t) : T(std::move(t)), valid(true) {}
    T& operator=(const T& t) = delete;
    T&& operator=(T&& t) = delete;
    Optional() : valid(false) {}
    virtual ~Optional() {};
    bool is_valid() { return valid; }
    operator bool() { return valid; }
    bool operator !() { return !valid; }
private:
    bool valid;
};




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
