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

    inline string to_str(s64 num, u8 base = 10) {
        char str[12];

        u8 i = 0;
        bool isNegative = false;

        /* Handle 0 explicitely, otherwise empty string is printed for 0 */
        if (num == 0)
            return "0";

        // In standard itoa(), negative numbers are handled only with
        // base 10. Otherwise numbers are considered unsigned.
        if (num < 0 && base == 10)
        {
            isNegative = true;
            num = -num;
        }

        // Process individual digits
        while (num != 0)
        {
            int rem = num % base;
            str[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
            num = num/base;
        }

        // If number is negative, append '-'
        if (isNegative)
            str[i++] = '-';

        str[i] = '\0'; // Append string terminator

        // Reverse the string
        u8 start = 0;
        u8 end = i -1;
        while (start < end)
        {

            auto tmp = *(str+start);
            *(str+start) = *(str+end);
            *(str+end) = tmp;
            start++;
            end--;
        }

        return str;
    }

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

    /**
     * @brief   Split string on delimiter, return only nonempty segments
     */
    template <typename C>
    C split_string(const string& s, char delimiter)
    {
        // define result holder
        C result;

        // segment begin, segment end
        string::const_iterator s_begin = s.begin();
        string::const_iterator s_end = s.begin();

        // scan the string till the end
        while (s_end != s.end())
        {
            // if delimiter found
            if (*s_end == delimiter)
            {
                // add the string segment to results
                if (s_begin != s_end)
                    result.push_back(string(s_begin, s_end));

                // move the begin past the delimiter
                s_begin = s_end + 1;
            }
            ++s_end;
        }

        // add remaining string segment and return
        if (s_begin != s_end)
            result.push_back(string(s_begin, s_end));
        return result;
    }

    template <typename C>
    string join_string(string separator, const C& elements) {
        if (elements.empty())
            return "";

        u32 len = 0;
        for (const string& s : elements)
            len += s.length();

        len += (elements.size() - 1) * separator.length();
        string result;
        result.reserve(len);

        for (u32 i = 0; i < elements.size() - 1; i++)
            result += elements[i] + separator;
        result += elements.back();

        return result;
    }
} // namespace kstd


#endif /* ELFS_TERMINAL_KSTD_H_ */
