/**
 *   @file: StringUtils.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_STRINGUTILS_H_
#define USER_USTD_SRC_STRINGUTILS_H_

#include <type_traits>
#include "ustd.h"
#include "types.h"
#include "Conversions.h"
#include "String.h"
#include "Vector.h"

namespace ustd {

// format helper - choose proper conversion to string depending on argument type
namespace {
template <class T>
struct FormatHelper {
    static string format(T what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<bool> {
    static string format(bool what) {
        return (what) ? "true" : "false";
    }
};

template <>
struct FormatHelper<float> {
    static string format(float what) {
        return conversions::double_to_string(what);
    }
};

template <>
struct FormatHelper<double> {
    static string format(double what) {
        return conversions::double_to_string(what);
    }
};

template <>
struct FormatHelper<string> {
    static string format(string what) {
        return what;
    }
};

template <>
struct FormatHelper<const char*> {
    static string format(const char* what) {
        return {what};
    }
};

template <>
struct FormatHelper<char*> {
    static string format(char* what) {
        return {what};
    }
};
}


class StringUtils {
public:
    static string from_int(s64 num, u8 base = 10);

    static string from_double(double num);

    static s64 to_int(const string& str);

    static double to_double(const string& str);

    static string to_lower_case(string s);

    static string to_upper_case(string s);

    static vector<string> split_string(const string& s, char delimiter);

    static string join_string(const string& separator, const vector<string>& elements);

    static string format(char const *fmt) {
        return {fmt};
    }

    template<typename Head, typename ... Tail>
    static string format(char const *fmt, Head& head, Tail ... tail) {
        using NakedHead = typename std::remove_cv<Head>::type;

        string result;
        while (*fmt) {
            if (*fmt == '%') {
                result += FormatHelper<NakedHead>::format(head);
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
