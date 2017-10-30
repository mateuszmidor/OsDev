/**
 *   @file: kstd.cpp
 *
 *   @date: Jun 9, 2017
 * @author: mateusz
 */

#include <algorithm>
#include "kstd.h"
#include "types.h"
//#include "KernelLog.h"

//using utils::KernelLog;

//static KernelLog& klog = KernelLog::instance();


namespace memory {
    extern void* kmalloc(size_t size);
    extern void kfree(void* address);
};

extern "C" size_t strlen(char const * str) {
    size_t result = 0;
    while (str[result]) {
        result++;
    }

    return result;
}

void* operator new(size_t size) {
    return memory::kmalloc(size);
}

void* operator new[](size_t size) {
    return memory::kmalloc(size);
}

void operator delete[](void* ptr) {
    memory::kfree(ptr);
}

void operator delete(void *ptr) {
    memory::kfree(ptr);
}

extern "C" void * memcpy( void * destination, const void * source, size_t num ) {
    char *dst = (char*)destination;
    const char *src = (const char*)source;
    for (int i = 0; i < num; i++)
        dst[i] = src[i];
    return dst;
}

extern "C" void * memmove( void * destination, const void * source, size_t num ) {
    return memcpy(destination, source, num);
}

extern "C" int memcmp ( const void * ptr1, const void * ptr2, size_t num ) {
    const char *p1 = (const char*)ptr1;
    const char *p2 = (const char*)ptr2;
    for (int i = 0; i < num; i++)
        if (p1[i] < p2[i])
            return -1;
        else if (p1[i] > p2[i])
            return 1;

    return 0;
}

extern "C" void* memset( void* dest, int ch, size_t count ) {
    char* dst = (char*)dest;
    for (size_t i = 0; i < count; i++)
        dst[i] = (char)ch;

    return dest;
}

extern "C" const void* memchr(const void* ptr, int ch, std::size_t count) {
    const char* data = (const char*)ptr;
    const char c = (char)ch;

    for (;count > 0; count--) {
        if (*data == c)
            return data;
        data++;
    }

    return nullptr;
}

//extern "C" void* memchr(void* ptr, int ch, std::size_t count) {
//    return memchr((const void*)ptr, ch, count);
//}

extern "C" void __cxa_pure_virtual() {
//    klog.format("pure virtual function called\n");
}

void std::__throw_length_error(char const* s) {
//    klog.format("__throw_length_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_logic_error(char const* s) {
//    klog.format("__throw_logic_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_alloc() {
//    klog.format("__throw_bad_alloc\n");
    __builtin_unreachable();
}

void std::__throw_out_of_range_fmt(char const* s, ...) {
//    klog.format("__throw_out_of_range_fmt: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_function_call() {
//    klog.format("__throw_bad_function_call\n");
    __builtin_unreachable();
}

namespace kstd {


string to_str(s64 num, u8 base) {
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

string to_upper_case(string s) {
    auto to_upper = [](char c) -> u32 {
        if (c >= 'a' && c <= 'z')
            return c + 'A' - 'a';
        else
            return c;
    };
    std::transform(s.begin(), s.end(), s.begin(), to_upper);
    return s;
}

void split_key_value(const string &kv, string &key, string &value, char separator) {
    auto pivot = kv.rfind(separator);
    key = kv.substr(0, pivot);
    value = kv.substr(pivot + 1, kv.length());
}

long long str_to_long(const char* str) {
    bool negative = false;
    if (str[0] == '-') {
        str++;
        negative = true;
    }

    long long res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    for (int i = 0; str[i] != '\0'; ++i) {
        unsigned char c = str[i];
        res = res*10 + c - '0';
            }

    return negative ? -res : res;
}

unsigned long long hex_to_long(const char* str) {

    str += 2; // skip 0x prefix
    unsigned long long res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    for (int i = 0; str[i] != '\0'; ++i) {
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

// build string from arrach of characters and trim non-writable characters on the right
string rtrim(const u8 *in, u16 len) {
    string s((const char*)in, len);
    auto pred = [] (u8 c) { return (c < 33 || c > 127); };
    auto last = std::find_if(s.begin(), s.end(), pred);
    return string (s.begin(), last);
}

string format(const string& fmt) {
    return fmt;
}

string format(s64 num) {
    return format(kstd::to_str(num));
}

string format(char const *fmt) {
    return string(fmt);
}

// cuts off head of the str that ends with "delimiter" or returns whole str if no "delimiter" found
// can return empty string in case str starts with delimiter
string snap_head(string& str, char delimiter) {
    auto pivot = str.find(delimiter);

    // last segment?
    if (pivot == string::npos)
        return std::move(str); // move clears str

    string result = str.substr(0, pivot);
    str = str.substr(pivot + 1, str.length());

    return result;
}

} // namespace kstd
