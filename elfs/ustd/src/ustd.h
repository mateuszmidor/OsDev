/*
 * ustd.h
 *
 *  Created on: Oct 20, 2017
 *      Author: mateusz
 */

#ifndef ELFS_TERMINAL_KSTD_H_
#define ELFS_TERMINAL_KSTD_H_

#include <string>
#include <vector>
#include "types.h"

namespace ustd {
    using string = std::string;
    template <class T>
    using vector = std::vector<T, std::allocator<T>>;

    string to_str(s64 num, u8 base = 10);
    vector<string> split_string(const string& s, char delimiter);
    string join_string(const string& separator, const vector<string>& elements);

    inline string format(const string& fmt) {
        return fmt;
    }

    inline string format(s64 num) {
        return format(ustd::to_str(num));
    }

    inline string format(char const *fmt) {
        return string(fmt);
    }



    template<typename Head, typename ... Tail>
    string format(char const *fmt, Head head, Tail ... tail) {

        string result;
        while (*fmt) {
            if (*fmt == '%') {
                result += format(head);
                result += format(++fmt, tail...);
                break;
            } else
                result += (*fmt++);
        }

        return result;
    }

    template<typename ... Args>
    string format(const string& fmt, Args ... args) {
        return format(fmt.c_str(), args...);
    }

} // namespace kstd


#endif /* ELFS_TERMINAL_KSTD_H_ */
