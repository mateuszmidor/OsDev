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





} // namespace kstd
