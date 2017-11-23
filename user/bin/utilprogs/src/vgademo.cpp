/**
 *   @file: vgademo.cpp
 *
 *   @date: Nov 15, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "VgaCharacter.h"

using namespace ustd;

void clear_screen(u16 height, u16 width) {
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            syscalls::vga_set_pixel_at(x, y, drivers::EgaColor::Black);
}

void draw_check_board(u16 height, u16 width, u16 size) {
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            auto color = (drivers::EgaColor)size + 1;
            if ((x / size + y / size) % 2)
                color = drivers::EgaColor::Black;

            syscalls::vga_set_pixel_at(x, y, color);
        }
}

void draw_ega64_color_palette(u16 height, u16 width) {
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 20; x++)
            syscalls::vga_set_pixel_at(x, y, y / 2);
}

/**
 * @brief   Entry point
 * @return  0 on success
 */
int main(int argc, char* argv[]) {
    u16 size = 40;

    u16 width, height;

    syscalls::vga_enter_graphics_mode();
    syscalls::vga_get_width_height(&width, &height);


    while (size > 0) {
        draw_check_board(height, width, size);
        size >>= 1;
        syscalls::msleep(1000);
    }

    draw_ega64_color_palette(height, width);
    syscalls::msleep(5000);

    syscalls::vga_exit_graphics_mode();

    cout::print("Vga demo done.");

    return 0;
}
