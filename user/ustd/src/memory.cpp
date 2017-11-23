/**
 *   @file: memory.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */
#include "types.h"

size_t bump_addr = 128*1024*1024;

void* umalloc(size_t size) {
    auto old_bump_addr = bump_addr;
    bump_addr+= size;
    return (void*)old_bump_addr;
}

void ufree(void* address) {
}


void* operator new(size_t size) {
    return umalloc(size);
}

void* operator new[](size_t size) {
    return umalloc(size);
}

void operator delete(void *ptr) noexcept {
    ufree(ptr);
}

void operator delete[](void* ptr) noexcept {
    ufree(ptr);
}

extern "C" void * memcpy( void * destination, const void * source, size_t num ) {
    char *dst = (char*)destination;
    const char *src = (const char*)source;
    for (size_t i = 0; i < num; i++)
        dst[i] = src[i];
    return dst;
}

extern "C" void * memmove( void * destination, const void * source, size_t num ) {
    return memcpy(destination, source, num);
}

extern "C" int memcmp( const void * ptr1, const void * ptr2, size_t num ) {
    const char *p1 = (const char*)ptr1;
    const char *p2 = (const char*)ptr2;
    for (size_t i = 0; i < num; i++)
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
