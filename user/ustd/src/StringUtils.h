/**
 *   @file: StringUtils.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_STRINGUTILS_H_
#define USER_USTD_SRC_STRINGUTILS_H_

#include "types.h"
#include "String.h"
#include "Vector.h"

namespace ustd {


template <class V>
struct Formatter;

template <>
struct Formatter<s64>  {
    static string format(s64 num) {
        return "integer";
    }
};

template <>
struct Formatter<u16>  {
    static string format(u16 num) {
        return "u16";
    }
};

template <>
struct Formatter<s32>  {
    static string format(s32 num) {
        return "s32";
    }
};

template <>
struct Formatter<u32>  {
    static string format(u32 num) {
        return "u32";
    }
};

template <>
struct Formatter<string>  {
    static string format(string num) {
        return "string";
    }
};

template <>
struct Formatter<char*>  {
    static string format(string num) {
        return "char*";
    }
};

template <>
struct Formatter<const char*>  {
    static string format(const char* num) {
        return "char*";
    }
};

template <>
struct Formatter<double>  {
    static string format(double num) {
        return "double";
    }
};



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

    static string format(const string& fmt) {
        return fmt;
    }

    static string format(s64 num) {
        return format(from_int(num));
    }

    static string format_double(double num) {
        return format(from_double(num));
    }

    static string format(char const *fmt) {
        return string(fmt);
    }

    template<typename ... Tail>
    static string format(char const *fmt, double head, Tail ... tail) {

        string result;
        while (*fmt) {
            if (*fmt == '%') {
                result += format_double(head);
                result += format(++fmt, tail...);
                break;
            } else
                result += (*fmt++);
        }

        return result;
    }

//#include  <type_traits>
    template<typename Head, typename ... Tail>
    static string format(char const *fmt, Head& head, Tail ... tail) {

//        using F = Formatter<typename std::remove_cv<Head>::type>;
        string result;
        while (*fmt) {
            if (*fmt == '%') {
                // F::for mat(head);
                result +=format(head);
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
