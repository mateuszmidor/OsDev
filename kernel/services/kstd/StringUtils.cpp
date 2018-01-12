/**
 *   @file: StringUtils.cpp
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#include <algorithm>
#include "StringUtils.h"

namespace kstd {

/**
 * @brief   Convert integer "num" to string using numeric system "base"
 */
string StringUtils::from_int(s64 num, u8 base) {
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
    while (start < end) {

        auto tmp = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = tmp;
        start++;
        end--;
    }

    return str;
}


/**
 * @brief   Convert decimal string to signed int
 */
s64 StringUtils::dec_to_int(const string& str) {
    bool negative = false;
    u32 start = 0;
    if (str[0] == '-') {
        start = 1;
        negative = true;
    }

    long long res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    for (u32 i = start; str[i] != '\0'; ++i) {
        unsigned char c = str[i];
        res = res*10 + c - '0';
    }

    return negative ? -res : res;
}

/**
 * @brief   Convert hexadecimal string to unsigned int
 */
u64 StringUtils::hex_to_uint(const string& str) {
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
 * @brief   Split "Key=Value" into "Key" and "Value" using given "separator"
 */
void StringUtils::split_key_value(const string &kv, string &key, string &value, char separator) {
    auto pivot = kv.rfind(separator);
    key = kv.substr(0, pivot);
    value = kv.substr(pivot + 1, kv.length());
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
 * @brief   Build string from array of characters and trim non-writable characters on the right
 */
string StringUtils::rtrim(const char* in, size_t len) {
    string s(in, len);
    auto pred = [] (u8 c) { return (c < 33 || c > 127); };
    auto last = std::find_if(s.begin(), s.end(), pred);
    return string (s.begin(), last);
}
} /* namespace kstd */
