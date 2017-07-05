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
#include "types.h"


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
using vector = std::vector<T, Allocator<T>>;

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

template <class T>
class Optional : public T {
public:
    Optional(const T& t) : T(t), valid(true) {}
    Optional(T&& t) : T(std::move(t)), valid(true) {}
    T& operator=(const T& t) = delete;
    T&& operator=(T&& t) = delete;
    Optional() : valid(false) {}
    virtual ~Optional() {};
    bool is_valid() { return valid; }

private:
    bool valid;
};

void split_key_value(const string &kv, string &key, string &value);
unsigned long long hex_to_long(const char* str);
string rtrim(const u8 *in, u16 len);

/**
    @example    flags_to_str(6, "READ=0x4", "WRITE=0x2", "EXEC=0x1");
                > READ WRITE
*/
inline string flags_to_str(unsigned long long flags) {
    return string();
}

template <class H, class ...T>
string flags_to_str(unsigned long long flags, H head, T... tail) {
    string k, v;
    split_key_value(head, k, v);
    unsigned long long val = hex_to_long(v.c_str());
    if ((flags & val) == val)
        return string(k) + " " + flags_to_str(flags, tail...);
    else
        return flags_to_str(flags, tail...);
}


/**
    @example    enum_to_str(3, "CLOSE=0x3", "READ=0x2", "OPEN=0x1");
                > CLOSE
*/
inline string enum_to_str(unsigned long long enumval) {
    return string("[UNKNOWN]");
}

template <class H, class ...T>
string enum_to_str(unsigned long long enumval, H head, T... tail) {
    string k, v;
    split_key_value(head, k, v);
    unsigned long long val = hex_to_long(v.c_str());
    if (val == enumval)
        return string(k);
    else
        return enum_to_str(enumval, tail...);
}

/**
 * @brief   Split string on delimiter, return only nonempty segments
 */
template <typename C>
C split_string(const string& s, char delimiter)
{
    // define result holder
    C result;

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

}; // namespace kstd

#endif /* SRC_KSTD_H_ */
