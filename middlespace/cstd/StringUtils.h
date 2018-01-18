/**
 *   @file: StringUtils.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_STRINGUTILS_H_
#define USER_USTD_SRC_STRINGUTILS_H_

#include <type_traits>
#include "types.h"
#include "String.h"
#include "Vector.h"
#include "Conversions.h"

namespace cstd {
namespace details {


// format helper - choose proper conversion to string depending on argument type
template <class T>
struct FormatHelper {
    static string format(T what) {
        return {what};
    }
};

template <>
struct FormatHelper<u8> {
    static string format(u8 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<s8> {
    static string format(s8 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<u16> {
    static string format(u16 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<s16> {
    static string format(s16 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<u32> {
    static string format(u32 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<s32> {
    static string format(s32 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<u64> {
    static string format(u64 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<s64> {
    static string format(s64 what) {
        return conversions::int_to_string(what);
    }
};

template <>
struct FormatHelper<long unsigned int> {
    static string format(long unsigned int what) {
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
    static string format(const string& what) {
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
} // namespace details


class StringUtils {
public:
    static string from_int(s64 num, u8 base = 10);

    static string from_double(double num, u8 max_frac_digits = 10);

    static s64 to_int(const string& str);

    static u64 to_hex(const string& str);

    static double to_double(const string& str);

    static string to_lower_case(string s);

    static string to_upper_case(string s);

    static vector<string> split_string(const string& s, char delimiter);

    static string join_string(const string& separator, const vector<string>& elements);

    static void split_key_value(const string &kv, string &key, string &value, char separator);

    static string snap_head(string& str, char delimiter);

    static string rtrim(const char* in, size_t len);



    static string format(char const *fmt) {
        return {fmt};
    }

    template<typename Head, typename ... Tail>
    static string format(char const *fmt, Head head, Tail ... tail) {
        using NakedHead = typename std::remove_cv<Head>::type;

        string result;
        while (*fmt) {
            if (*fmt == '%') {
                result += details::FormatHelper<NakedHead>::format(head);
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
        unsigned long long val = to_hex(v.c_str());
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
        unsigned long long val = to_hex(v.c_str());
        if (val == enumval)
            return string(k);
        else
            return enum_to_str(enumval, tail...);
    }
};

} /* namespace cstd */

#endif /* USER_USTD_SRC_STRINGUTILS_H_ */
