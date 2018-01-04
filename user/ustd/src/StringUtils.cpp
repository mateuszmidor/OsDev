/**
 *   @file: StringUtils.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "StringUtils.h"

namespace ustd {

/**
 * @brief   Convert integer "num" to string using numeric system "base"
 */
string StringUtils::from_int(s64 num, u8 base) {
    return conversions::int_to_string(num, base);
}

/**
 * @brief   Convert integer "num" to string using numeric system "base"
 */
string StringUtils::from_double(double num, u8 max_frac_digits) {
    return conversions::double_to_string(num, max_frac_digits);
}

/**
 * @brief   Convert string to long
 */
s64 StringUtils::to_int(const string& str) {
    return conversions::string_to_int(str);
}

/**
 * @brief   Convert string to double
 */
double StringUtils::to_double(const string& str) {
    return conversions::string_to_double(str);
}

/**
 * @brief   Change string to lower case
 */
string StringUtils::to_lower_case(string s) {
    auto to_upper = [](char c) -> u32 {
        if (c >= 'A' && c <= 'Z')
            return c - ('A' - 'a');
        else
            return c;
    };
    std::transform(s.begin(), s.end(), s.begin(), to_upper);
    return s;
}

/**
 * @brief   Change string to upper case
 */
string StringUtils::to_upper_case(string s) {
    auto to_upper = [](char c) -> u32 {
        if (c >= 'a' && c <= 'z')
            return c + ('A' - 'a');
        else
            return c;
    };
    std::transform(s.begin(), s.end(), s.begin(), to_upper);
    return s;
}

/**
 * @brief   Split string on delimiter, return only nonempty segments
 */
vector<string> StringUtils::split_string(const string& s, char delimiter) {
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
string StringUtils::join_string(const string& separator, const vector<string>& elements) {
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



} /* namespace ustd */
