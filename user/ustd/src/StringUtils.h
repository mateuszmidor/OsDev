/**
 *   @file: StringUtils.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_STRINGUTILS_H_
#define USER_USTD_SRC_STRINGUTILS_H_

#include "String.h"
#include "Vector.h"

namespace ustd {

class StringUtils {
public:
    static string from_int(s64 num, u8 base = 10);

    static s64 to_int(const string& str);

    static string to_lower_case(string s);

    static vector<string> split_string(const string& s, char delimiter);

    static string join_string(const string& separator, const vector<string>& elements);

    static string format(const string& fmt) {
        return fmt;
    }

    static string format(s64 num) {
        return format(from_int(num));
    }

    static string format(char const *fmt) {
        return string(fmt);
    }

    template<typename Head, typename ... Tail>
    static string format(char const *fmt, Head head, Tail ... tail) {

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
    static string format(const string& fmt, Args ... args) {
        return format(fmt.c_str(), args...);
    }
};

} /* namespace ustd */

#endif /* USER_USTD_SRC_STRINGUTILS_H_ */
