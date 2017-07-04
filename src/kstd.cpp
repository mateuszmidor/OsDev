/**
 *   @file: kstd.cpp
 *
 *   @date: Jun 9, 2017
 * @author: mateusz
 */

#include <algorithm>
#include "kstd.h"
#include "types.h"
#include "ScreenPrinter.h"

static ScreenPrinter &printer = ScreenPrinter::instance();

// allocation should be done using memory manager
void* bump_alloc(size_t size) {
    static size_t addr = 2 * 1024 * 1024;

    size_t old = addr;
    addr += size;
   // printer.format("Allocated % bytes of mem\n", size);
    return (void*)old;
}

void* operator new(size_t size) {
    return bump_alloc(size);
}

void* operator new[](size_t size) {
    return bump_alloc(size);
}

void operator delete[](void* ptr) {
    // should deallocate using memory manager
}

void operator delete(void *ptr) {
    // should deallocate using memory manager
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

extern "C" void __cxa_pure_virtual() {
    printer.format("pure virtual function called\n");
}

void std::__throw_length_error(char const* s) {
    printer.format("__throw_length_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_logic_error(char const* s) {
    printer.format("__throw_logic_error: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_alloc() {
    printer.format("__throw_bad_alloc\n");
    __builtin_unreachable();
}

void std::__throw_out_of_range_fmt(char const* s, ...) {
    printer.format("__throw_out_of_range_fmt: %\n", s);
    __builtin_unreachable();
}

void std::__throw_bad_function_call() {
    printer.format("__throw_bad_function_call\n");
    __builtin_unreachable();
}

namespace kstd {

void split_key_value(const string &kv, string &key, string &value) {
    auto pivot = kv.rfind("=");
    key = kv.substr(0, pivot);
    value = kv.substr(pivot + 1, kv.length());
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

} // namespace kstd
