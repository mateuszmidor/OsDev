/**
 *   @file: MouseDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "MouseDriver.h"

namespace drivers {

MouseDriver::MouseDriver() {
    setup();
}

MouseDriver::~MouseDriver() {
}

void MouseDriver::on_interrupt() {
    // check for mouse data available
    u8 status = mouse_cmd_port.read();
    if (!(status & 0x01) || !(status & 0x20))
        return;

    // read data sequence chunk (one of three chunks)
    buffer[offset] = mouse_data_port.read();
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
                on_down(i);

            if (!(buffer[0] & (1 << i)) && (buttons & (1 << i)))
                on_up(i);
        }

        // remember button state
        buttons = buffer[0];
    }
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

bool MouseDriver::setup() {
    mouse_cmd_port.write(0xA8);
    mouse_cmd_port.write(0x20);
    u8 status = mouse_data_port.read() | 0x02;
    mouse_cmd_port.write(0x60);
    mouse_data_port.write(status);

    mouse_cmd_port.write(0xD4);
    mouse_data_port.write(0xF4);

    return (mouse_data_port.read() == 0xFA);
}
} /* namespace drivers */
