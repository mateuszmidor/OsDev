/**
 *   @file: memory.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */
#include "types.h"
#include "syscalls.h"
#include "ScopeGuard.h"

size_t bump_addr = 0;
size_t bump_old_limit = 0;
size_t bump_limit = 0;
ustd::Mutex umalloc_mutex;

void* umalloc(size_t size) {
    ustd::ScopeGuard one_thread_at_a_time_here(umalloc_mutex);

    // setup dynamic memory allocation start
    if (bump_addr == 0) {
        bump_addr = syscalls::brk(0);
        bump_limit = bump_addr; // we start with 0 bytes of dynamic memory and then alloc as needed
    }

    // alloc dynamic memory to the program if needed
    if (bump_addr + size > bump_limit) {
        size_t increase = size > 1024*1024 ? size : 1024*1024;
        bump_old_limit = bump_limit;
        bump_limit = syscalls::brk(bump_old_limit + increase);
    }

    // if allocation failed - out of memory
    if (bump_limit == bump_old_limit)
        return nullptr;

    // cut out chunk of program dynamic memory
    size_t old_bump_addr = bump_addr;
    bump_addr+= size;
    return (void*)old_bump_addr;
}

void ufree(void* address) {
    // bump allocator doesnt free
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

extern "C" const void* memchr(const void* ptr, int ch, size_t count) {
    const char* data = (const char*)ptr;
    const char c = (char)ch;

    for (;count > 0; count--) {
        if (*data == c)
            return data;
        data++;
    }

    return nullptr;
}
