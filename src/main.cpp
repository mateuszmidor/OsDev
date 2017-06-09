/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "kstd.h"


using namespace kstd;

void test_kstd(ScreenPrinter &p) {
    // test vector
    vector<long long> vec;
    for (int i = 0; i < 10; i++)
        vec.push_back(i);
    for (auto a : vec)
        p.format("%, ", a);

    p.format("\n");

    // test sting
    string s("abcdefghijklkmnoprstuwxyz");
    s += "123456abcdefghijklkmnoprstuwxyz";
    p.format("%\n", s.c_str());

    // test flags
//    string s2= flags_to_str(5, "READ=0x4", "WRITE=0x2", "EXEC=0x1");
//    p.format("%\n", s2.c_str());
}


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
    mb2.print(p);

    //test_kstd(p);
}
