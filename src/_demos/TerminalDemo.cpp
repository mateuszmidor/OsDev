/*
 * TerminalDemo.cpp
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#include "TerminalDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"

using namespace drivers;
namespace demos {

void TerminalDemo::run() {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();

    auto vga = driver_manager.get_driver<VgaDriver>();
    if (!vga) {
        klog.format("VgaDemo::run: no VgaDriver\n");
        return;
    }

    auto keyboard = driver_manager.get_driver<KeyboardDriver>();
    if (!keyboard) {
        klog.format("VgaDemo::run: no KeyboardDriver\n");
        return;
    }

    keyboard->set_on_key_press([&](Key key) { on_key_press(key); });
}

void TerminalDemo::on_key_press(Key key) {
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up:
            printer.scroll_up(1);
            break;

        case Key::Down:
            printer.scroll_down(1);
            break;

        case Key::PgUp:
            printer.scroll_up(4);
            break;

        case Key::PgDown:
            printer.scroll_down(4);
            break;

        case Key::Home:
            printer.scroll_to_begin();
            break;

        case Key::End:
            printer.scroll_to_end();
            break;

        case Key::Esc:
            printer.clear_screen();
            break;

        default:;
        }
    }
    else {
        char s[2] = {(char)key, '\0'};
        printer.format("%", s);
    }
}
} /* namespace demos */
