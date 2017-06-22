/**
 *   @file: kstd.cpp
 *
 *   @date: Jun 9, 2017
 * @author: mateusz
 */

#include "kstd.h"
#include "types.h"
#include "ScreenPrinter.h"

extern ScreenPrinter printer;

// allocation should be done using memory manager
void* bump_alloc(size_t size) {
    static size_t addr = 2 * 1024 * 1024;
    const u16 align = 8;

    size_t old = addr;
    addr += size;
    addr = (addr + (align-1)) / align * align; // next aligned address
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
//    ScreenPrinter p;
//    p.format("ERROR:%", s);
    __builtin_unreachable();
}

void std::__throw_logic_error(char const* s) {
//    ScreenPrinter p;
//    p.format("ERROR:%", s);
    __builtin_unreachable();
}

void std::__throw_bad_alloc() {
//    ScreenPrinter p;
//    p.format("BAD ALLOC");
    __builtin_unreachable();
}

void std::__throw_out_of_range_fmt(char const*, ...) {
//    ScreenPrinter p;
//    p.format("OUT OF RANGE");
    __builtin_unreachable();
}

void std::__throw_bad_function_call() {
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

} // namespace kstd
