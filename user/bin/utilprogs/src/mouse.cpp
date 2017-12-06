/**
 *   @file: mouse.cpp
 *
 *   @date: Dec 5, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "syscalls.h"
#include "Cout.h"
#include "Mouse.h"
#include "Vga.h"

using namespace ustd;
using namespace middlespace;


constexpr u16 screen_width_in_pixels = 720;
constexpr u16 screen_height_in_pixels = 400;
u16 screen_width_in_chars;
u16 screen_height_in_chars;
u8 char_width;
u8 char_height;

s16 mouse_x_in_pixels = screen_width_in_pixels  / 2;
s16 mouse_y_in_pixels = screen_height_in_pixels / 2;



s16 clamp(s16 v, s16 min, s16 max) {
    if (v < min) v = min;
    if (v > max) v = max;
    return v;
}

void swap_fg_bg_at(u16 x_in_pixels, u16 y_in_pixels) {
    u16 x = x_in_pixels / char_width;
    u16 y = y_in_pixels / char_height;

    VgaCharacter c = syscalls::vga_get_char_at(x, y);
    VgaCharacter swapped_c = VgaCharacter {.character = (u8)c.character, .fg_color = c.bg_color, .bg_color = c.fg_color};
    syscalls::vga_set_char_at(x, y, swapped_c);
}

void handle_mouse(const MouseState& mouse_state) {
    if (!mouse_state.buttons[MouseButton::LEFT])
        swap_fg_bg_at(mouse_x_in_pixels, mouse_y_in_pixels);

    mouse_x_in_pixels = clamp(mouse_x_in_pixels + mouse_state.dx, 0, screen_width_in_pixels - char_width);
    mouse_y_in_pixels = clamp(mouse_y_in_pixels + mouse_state.dy, 0, screen_height_in_pixels - char_height);

    swap_fg_bg_at(mouse_x_in_pixels, mouse_y_in_pixels);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    int fd = syscalls::open("/dev/mouse", 2);
    if (fd < 0) {
        cout::print("mouse: cant open /dev/mouse\n");
        return 1;
    }

    cout::print("mouse: Move cursor to the left top corner to exit.\n");

    syscalls::vga_get_width_height(&screen_width_in_chars, &screen_height_in_chars);
    char_width = screen_width_in_pixels / screen_width_in_chars;
    char_height = screen_height_in_pixels / screen_height_in_chars;

    MouseState mouse_state;
    while (syscalls::read(fd, &mouse_state, sizeof(MouseState)) == sizeof(MouseState)) {
        handle_mouse(mouse_state);
        if (mouse_x_in_pixels == 0 && mouse_y_in_pixels == 0) break;
    }

    cout::print("mouse: exit.\n");
    syscalls::close(fd);

    return 0;
}
