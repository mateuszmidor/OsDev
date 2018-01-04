/**
 *   @file: Conversions.cpp
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#include "Conversions.h"

namespace ustd {
namespace conversions {

/**
 * @brief   Convert integer "num" to string using numeric system "base"
 */
string int_to_string(s64 num, u8 base) {
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
 * @brief   Convert integer "num" to string using numeric system "base"
 * @note    Ugly and fixed-fract digits count implementation
 */
string double_to_string(double num, u8 max_frac_digits) {
    const auto FRAC_DIGITS_LIMIT = 10;
    char str[30];
    const int base = 10;
    u8 i = 0;
    bool isNegative = false;

    if (max_frac_digits > FRAC_DIGITS_LIMIT)
        max_frac_digits = FRAC_DIGITS_LIMIT;

    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if (num == 0.0)
        return "0";

    if (num < 0.0)
    {
        isNegative = true;
        num = -num;
    }

    double fnum = num - (ssize_t)num;
    for (i = 0; i < max_frac_digits; i++) {
        fnum *=10;
        int rem = int(fnum) % base;
        str[max_frac_digits-i-1] = rem + '0';
    }

    str[i++] = '.';


    long long inum = (long long)num;

    // Process individual digits
    if (inum == 0)
        str[i++] = '0';
    else
        while (inum)
        {
            int rem = inum % base;
            str[i++] = rem + '0';
            inum = inum/base;
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
 * @brief   Convert string to long
 */
s64 string_to_int(const string& str) {
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
 * @brief   Convert string to double
 */
double string_to_double(const string& str) {
    bool negative = false;
    u32 start = 0;
    if (str[0] == '-') {
        start = 1;
        negative = true;
    }

    long long res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    size_t i;
    for (i = start; (str[i] != '\0') && (str[i] != '.'); ++i) {
        unsigned char c = str[i];
        res = res*10 + c - '0';
    }

    if (str[i] == '\0')
        return negative ? -res : res;

    // got period
    i++;
    long long frac = 0;
    size_t len = 1;
    for (; str[i] != '\0'; ++i) {
        unsigned char c = str[i];
        frac = frac*10 + c - '0';
        len *= 10;
    }

    double result = res + frac / (double)len;
    return negative ? -result : result;
}
};
} /* namespace ustd */
