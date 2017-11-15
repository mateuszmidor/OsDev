/**
 *   @file: MouseDemo.cpp
 *
 *   @date: Jul 21, 2017
 * @author: Mateusz Midor
 */

#include "MouseDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "MouseDriver.h"
#include "SysCallNumbers.h"

using namespace drivers;
using namespace middlespace;
using logging::KernelLog;

namespace demos {

void MouseDemo::run(u64 arg) {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();

    vga = driver_manager.get_driver<VgaDriver>();
    if (!vga) {
        klog.format("MouseDemo::run: no VgaDriver\n");
        return;
    }

    auto mouse = driver_manager.get_driver<MouseDriver>();
    if (!mouse) {
        klog.format("MouseDemo::run: no MouseDriver\n");
        return;
    }

    mouse->set_on_move([&](s8 dx, s8 dy) { on_mouse_move(dx, dy); });
    mouse->set_on_down([&](u8 button) { on_mouse_down(button); });

    // keep task alive
    while (true)
        asm volatile("int $0x80" : : "a"(middlespace::Int80hSysCallNumbers::NANOSLEEP));    // yield
}

void MouseDemo::on_mouse_down(u8 button) {
    const u8 CHAR_WIDTH = 720 / vga->screen_width();
    const u8 CHAR_HEIGHT = 400 / vga->screen_height();
    //printer.move_to(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
}

void MouseDemo::on_mouse_move(s8 dx, s8 dy) {
    const u8 CHAR_WIDTH = 720 / vga->screen_width();
    const u8 CHAR_HEIGHT = 400 / vga->screen_height();

    swap_fg_bg_at(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
    mouse_x += dx ;
    mouse_y += dy;
    if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0; if (mouse_x > 719) mouse_x = 719; if (mouse_y > 399) mouse_y = 399;
    swap_fg_bg_at(mouse_x / CHAR_WIDTH, mouse_y / CHAR_HEIGHT);
}

void MouseDemo::swap_fg_bg_at(u16 x, u16 y) {
    VgaCharacter c = vga->at(x, y);
    vga->at(x, y) = VgaCharacter {.character = (u8)c.character, .fg_color = c.bg_color, .bg_color = c.fg_color};
}
} /* namespace demos */
