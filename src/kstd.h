/**
 *   @file: kstd.h
 *
 *   @date: Jun 8, 2017
 * @author: Mateusz Midor
 *   @note: This file provides minimum of stdlib functionality
 */

#ifndef SRC_KSTD_H_
#define SRC_KSTD_H_


#include <limits>
#include <vector>
#include <string>


void* operator new(size_t size) {
    // allocation should be done using memory manager
    return (void*)(2 * 1024 * 1024); // allocate at 2MB physical address
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


void std::__throw_length_error(char const* s) {
    ScreenPrinter p;
    p.format("ERROR:%", s);
}

void std::__throw_logic_error(char const* s) {
    ScreenPrinter p;
    p.format("ERROR:%", s);
}

void std::__throw_bad_alloc() {
    ScreenPrinter p;
    p.format("BAD ALLOC");
}

namespace kstd {

template <class T>
class Allocator {
public :
    //    typedefs
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public :
    //    convert an allocator<T> to allocator<U>
    template<typename U>
    struct rebind {
        typedef Allocator<U> other;
    };

public :
    inline explicit Allocator() {}
    inline ~Allocator() {}
    inline explicit Allocator(Allocator const&) {}
    template<typename U>
    inline explicit Allocator(Allocator<U> const&) {}

    //    address
    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    //    memory allocation
    inline pointer allocate(size_type cnt) {
      return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T)));
    }

    inline void deallocate(pointer p, size_type) {
        ::operator delete(p);
    }

    //    size
    inline size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    //    construction/destruction
    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

    inline bool operator==(Allocator const&) { return true; }
    inline bool operator!=(Allocator const& a) { return !operator==(a); }
};    //    end of class Allocator



template <class T>
using vector = std::vector<T>;

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;



}; // namespace kstd

#endif /* SRC_KSTD_H_ */
