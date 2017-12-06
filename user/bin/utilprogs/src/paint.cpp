/**
 *   @file: paint.cpp
 *
 *   @date: Dec 6, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cin.h"
#include "Cout.h"
#include "MouseState.h"
#include "VgaCharacter.h"

using namespace ustd;
using namespace drivers;
using namespace middlespace;


u16 screen_width_in_pixels;
u16 screen_height_in_pixels;

s16 mouse_x_in_pixels;
s16 mouse_y_in_pixels;



s16 clamp(s16 v, s16 min, s16 max) {
    if (v < min) v = min;
    if (v > max) v = max;
    return v;
}


void handle_mouse(const MouseState& mouse_state) {
    mouse_x_in_pixels = clamp(mouse_x_in_pixels + mouse_state.dx, 0, screen_width_in_pixels - 1);
    mouse_y_in_pixels = clamp(mouse_y_in_pixels + mouse_state.dy, 0, screen_height_in_pixels - 1);

    EgaColor pen_color = EgaColor::Black;
    if (mouse_state.buttons[MouseButton::LEFT])
        pen_color = EgaColor::Red;
    if (mouse_state.buttons[MouseButton::MIDDLE])
        pen_color = EgaColor::Yellow;
    if (mouse_state.buttons[MouseButton::RIGHT])
        pen_color = EgaColor::Green;

    syscalls::vga_set_pixel_at(mouse_x_in_pixels, mouse_y_in_pixels, pen_color);
}

void clear_screen() {
    for (u16 y = 0; y < screen_height_in_pixels; y++)
        for (u16 x = 0; x < screen_width_in_pixels; x++)
            syscalls::vga_set_pixel_at(x, y, EgaColor::Black);
}
/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    int fd = syscalls::open("/dev/mouse", 2);
    if (fd < 0) {
        cout::print("paint: cant open /dev/mouse\n");
        return 1;
    }

    cout::print("paint: Move cursor to the left top corner to exit. Press ENTER...\n");
    cin::readln();

    syscalls::vga_enter_graphics_mode();
    syscalls::vga_get_width_height(&screen_width_in_pixels, &screen_height_in_pixels);
    clear_screen();
    mouse_x_in_pixels = screen_width_in_pixels / 2;
    mouse_y_in_pixels = screen_height_in_pixels / 2;

    MouseState mouse_state;
    while (syscalls::read(fd, &mouse_state, sizeof(MouseState)) == sizeof(MouseState)) {
        handle_mouse(mouse_state);
        if (mouse_x_in_pixels == 0 && mouse_y_in_pixels == 0) break;
    }

    syscalls::vga_exit_graphics_mode();
    cout::print("paint: exit.\n");
    syscalls::close(fd);

    return 0;
}
