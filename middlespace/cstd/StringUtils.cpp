/**
 *   @file: StringUtils.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "StringUtils.h"


namespace cstd {

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
 * @brief   Convert hexadecimal string to unsigned int
 */
u64 StringUtils::to_hex(const string& str) {
    unsigned long long res = 0; // Initialize result

    // Iterate through all characters of input string and compute number
    for (int i = 2; str[i] != '\0'; ++i) {  // i = 2 to skip 0x prefix
        unsigned char c = str[i];
        if (c >= '0' && c <= '9')
            res = res*16 + c - '0';
        else
        if (c >= 'A' && c <= 'F')
            res = res*16 + c - 'A' + 10;
        else
        if (c >= 'a' && c <= 'f')
            res = res*16 + c - 'a' + 10;
    }

    return res;
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

/**
 * @brief   Split "Key=Value" into "Key" and "Value" using given "separator"
 */
void StringUtils::split_key_value(const string &kv, string &key, string &value, char separator) {
    auto pivot = kv.rfind(separator);
    key = kv.substr(0, pivot);
    value = kv.substr(pivot + 1, kv.length());
}

/**
 * @brief   Cut off head of the str that ends with "delimiter" or return whole str if no "delimiter" found
 * @note    Can return empty string in case str starts with delimiter
 */
string StringUtils::snap_head(string& str, char delimiter) {
    auto pivot = str.find(delimiter);

    // last segment?
    if (pivot == string::npos)
        return std::move(str); // move clears str

    string result = str.substr(0, pivot);
    str = str.substr(pivot + 1, str.length());

    return result;
}

/**
 * @brief   Cut off tail of the str that starts with "delimiter" or return whole str if no "delimiter" found
 * @note    Can return empty string in case str ends with delimiter
 */
string StringUtils::snap_tail(string& str, char delimiter) {
    auto pivot = str.rfind(delimiter);

    // last segment?
    if (pivot == string::npos)
        return std::move(str); // move clears str

    string result = str.substr(pivot+1, str.length()-1);
    str = str.substr(0, pivot);

    return result;
}

/**
 * @brief   Build string from array of characters and trim non-writable characters on the right
 */
string StringUtils::rtrim(const char* in, size_t len) {
    string s(in, len);
    auto pred = [] (u8 c) { return (c < 33 || c > 127); };
    auto last = std::find_if(s.begin(), s.end(), pred);
    return string (s.begin(), last);
}

} /* namespace ustd */
