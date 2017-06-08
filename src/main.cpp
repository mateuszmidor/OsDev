/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include <cstddef>
#include <limits>
#include <vector>
#include <string>

void* operator new( size_t size) noexcept{
    return (void*)(639*1024);
}

void operator delete(void* ptr) {}

size_t strlen(const char *s) {
    if (s == nullptr)
        return 0;

    if (*s == '\0')
        return 0;

    size_t len = 0;
    while (s[++len]) ;
    return len;
}

extern "C" void * memcpy ( void * destination, const void * source, size_t num ) {
    char *dst = (char*)destination;
    const char *src = (const char*)source;
    for (int i = 0; i < num; i++)
        dst[i] = src[i];
    return dst;
}

namespace std {
void * memcpy ( void * destination, const void * source, size_t num ) {
    char *dst = (char*)destination;
    const char *src = (const char*)source;
    for (int i = 0; i < num; i++)
        dst[i] = src[i];
    return dst;
}

[[ noreturn ]] void __throw_length_error(char const* s) {
    ScreenPrinter p;
    p.format("ERROR:%", s);
}

[[ noreturn ]] void __throw_logic_error(char const* s) {
    ScreenPrinter p;
    p.format("ERROR:%", s);

}
}
template<typename T>
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


struct m_char_traits : public std::char_traits<char>
{

    typedef char _Elem;
    static std::size_t length(const _Elem *_Str)
    {
        return strlen(_Str);
    }
    static int compare(const _Elem *_Lhs, const _Elem *_Rhs, std::size_t _Count)
    {
        return 0;//strncmp(_Lhs, _Rhs, _Count);
    }
};


/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    ScreenPrinter p;
    p.set_bg_color(Color::Blue);
    p.format("\n\n"); // go to the third line of console as 1 and 2 are used upon initializing in 32 and then 64 bit mode
    p.format("Hello in kmain() of main.cpp!\n");

    CpuInfo cpu_info;
    cpu_info.print(p);

    Multiboot2 mb2(multiboot2_info_ptr);
//    mb2.print(p);
    std::vector<int, Allocator<int>> vec{100, 6, 5, 9};
    for (auto a : vec)
        p.format("%, ", a);

    using mystring = std::basic_string<char, m_char_traits, Allocator<char>>;
    mystring s("abcdefghijklkmnoprstuwxyz_");
    s += "123456abcdefghijklkmnoprstuwxyz";
    p.format("%", s.c_str());
}
