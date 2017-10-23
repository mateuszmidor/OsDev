/**
 *   @file: main.cpp
 *
 *   @date: Sep 20, 2017
 * @author: Mateusz Midor
 */


#include "../_start.h"
#include "../utils.h"
#include "Terminal.h"


extern "C" int memcmp ( const void * ptr1, const void * ptr2, size_t num ) {
    const char *p1 = (const char*)ptr1;
    const char *p2 = (const char*)ptr2;
    for (size_t i = 0; i < num; i++)
        if (p1[i] < p2[i])
            return -1;
        else if (p1[i] > p2[i])
            return 1;

    return 0;
}

//extern "C" void* memset( void* dest, int ch, size_t count ) {
//    char* dst = (char*)dest;
//    for (size_t i = 0; i < count; i++)
//        dst[i] = (char)ch;
//
//    return dest;
//}

//extern "C" const void* memchr(const void* ptr, int ch, std::size_t count) {
//    const char* data = (const char*)ptr;
//    const char c = (char)ch;
//
//    for (;count > 0; count--) {
//        if (*data == c)
//            return data;
//        data++;
//    }
//
//    return nullptr;
//}

using namespace terminal;
int main(int argc, char* argv[]) {
    Terminal t(0);
    t.run();
    print("Goodbye terminal");
}
