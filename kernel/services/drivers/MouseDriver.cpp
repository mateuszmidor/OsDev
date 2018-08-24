/**
 *   @file: MouseDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "MouseDriver.h"

using namespace hardware;

namespace drivers {

MouseDriver::MouseDriver() {
    setup();
}

MouseDriver::~MouseDriver() {
}

s16 MouseDriver::handled_interrupt_no() {
    return Interrupts::Mouse;
}

CpuState* MouseDriver::on_interrupt(CpuState* cpu_state) {
    handle_mouse_interrupt();
    return cpu_state;
}

void MouseDriver::set_on_move(const MouseMoveEvent &event) {
    on_move = event;
}

void MouseDriver::set_on_down(const MouseButtonEvent &event) {
    on_down = event;
}

void MouseDriver::set_on_up(const MouseButtonEvent &event) {
    on_up = event;
}

/**
 * @brief   Configure PS2 hardware and setup the mouse.
 * @see     https://wiki.osdev.org/Mouse_Input
 */
bool MouseDriver::setup() {
    // Set Compaq Status/Enable IRQ12
    mouse_cmd_port_64.write(0x20);
    u8 status = mouse_data_port_60.read();
    status |= 0x02;     // enable IRQ12
    status &= (~0x20);  // disable mouse clock
    mouse_cmd_port_64.write(0x60);
    mouse_data_port_60.write(status);

    // Aux Input Enable Command
    mouse_cmd_port_64.write(0xA8);

    // enable mouse movement packets generation
    mouse_cmd_port_64.write(0xD4);
    mouse_data_port_60.write(0xF4);

    return (mouse_data_port_60.read() == 0xFA);
}

void MouseDriver::handle_mouse_interrupt() {
    // check for mouse data available
    u8 status = mouse_cmd_port_64.read();
    if (!(status & 0x01) || !(status & 0x20))
        return;

    // read data sequence chunk (one of three chunks)
    buffer[offset] = mouse_data_port_60.read();
    offset = (offset + 1) % 3;

    // data sequence ended; generate events
    if (offset == 0) {
        // detect mouse movement
        if ((buffer[1] != 0) || (buffer[2] != 0)) {
            s8 dx = (buffer[1] - ((buffer[0] << 4) & 0x100));
            s8 dy = -(buffer[2] - ((buffer[0] << 3) & 0x100));
            on_move(dx, dy);

        }

        // detect button state change
        for (u8 i = 0; i < 3; i++) {
            if ((buffer[0] & (1 << i)) && !(buttons & (1 << i)))
                on_down((middlespace::MouseButton)i);

            if (!(buffer[0] & (1 << i)) && (buttons & (1 << i)))
                on_up((middlespace::MouseButton)i);
        }

        // remember button state
        buttons = buffer[0];
    }
}
} /* namespace drivers */
