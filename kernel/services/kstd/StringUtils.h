/**
 *   @file: StringUtils.h
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_KSTD_STRINGUTILS_H_
#define KERNEL_KSTD_STRINGUTILS_H_

#include "types.h"
#include "String.h"
#include "Vector.h"

namespace kstd {

class StringUtils {
public:
    static string from_int(s64 num, u8 base = 10);

    static s64 dec_to_int(const string& str);

    static u64 hex_to_uint(const string& str);

    static string to_lower_case(string s);

    static string to_upper_case(string s);

    static void split_key_value(const string &kv, string &key, string &value, char separator);

    static vector<string> split_string(const string& s, char delimiter);

    static string join_string(const string& separator, const vector<string>& elements);

    static string snap_head(string& str, char delimiter);

    static string rtrim(const char* in, size_t len);

    static string format(const string& fmt) {
        return fmt;
    }

    static string format(s64 num) {
        return format(from_int(num));
    }

    static string format(char const *fmt) {
        return string(fmt);
    }

    template<typename ... Args>
    static string format(const string& fmt, Args ... args) {
        return format(fmt.c_str(), args...);
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

    /**
        @example    flags_to_str(6, "READ=0x4", "WRITE=0x2", "EXEC=0x1");
                    > READ WRITE
    */
    static string flags_to_str(unsigned long long flags) {
        return string();
    }

    template <class H, class ...T>
    static string flags_to_str(unsigned long long flags, H head, T... tail) {
        string k, v;
        split_key_value(head, k, v, '=');
        unsigned long long val = hex_to_uint(v.c_str());
        if ((flags & val) == val)
            return string(k) + " " + flags_to_str(flags, tail...);
        else
            return flags_to_str(flags, tail...);
    }


    /**
        @example    enum_to_str(3, "CLOSE=0x3", "READ=0x2", "OPEN=0x1");
                    > CLOSE
    */
    static string enum_to_str(unsigned long long enumval) {
        return StringUtils::format("[?-%]", enumval);
    }

    template <class H, class ...T>
    static string enum_to_str(unsigned long long enumval, H head, T... tail) {
        string k, v;
        split_key_value(head, k, v, '=');
        unsigned long long val = hex_to_uint(v.c_str());
        if (val == enumval)
            return string(k);
        else
            return enum_to_str(enumval, tail...);
    }
};

} /* namespace kstd */

#endif /* KERNEL_KSTD_STRINGUTILS_H_ */
