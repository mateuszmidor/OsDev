/**
 *   @file: ustd.cpp
 *
 *   @date: Oct 28, 2017
 * @author: Mateusz Midor
 */

#include "ustd.h"

namespace ustd {

/**
 * @brief   Convert integer "num" to string using numeric system "base"
 */
inline string to_str(s64 num, u8 base) {
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

/**
 * @brief   Split string on delimiter, return only nonempty segments
 */
vector<string> split_string(const string& s, char delimiter) {
    // define result holder
    vector<string> result;

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

/**
 * @brief   Join strings using "separator"
 */
string join_string(const string& separator, const vector<string>& elements) {
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

}; // namespace ustd
