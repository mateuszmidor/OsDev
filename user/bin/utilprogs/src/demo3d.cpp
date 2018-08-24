/**
 *   @file: demo3d.cpp
 *
 *   @date: Aug 24, 2018
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "VgaDevice.h"

using namespace middlespace;
using namespace cstd::ustd;

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    VgaDevice vga;

    for (int y = 0; y < vga.height; y++)
        for (int x = 0; x < vga.width; x++) {
            vga.set_pixel_at(x, y, EgaColor64::LightPink);
            vga.flush_to_screen();
        }


    return 0;
}
