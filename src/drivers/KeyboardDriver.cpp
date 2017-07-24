/**
 *   @file: KeyboardDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "KeyboardDriver.h"

using namespace hardware;

namespace drivers {

/**
 * Constructor.
 * @param scs   KeyboardScanCodeSet translates scan codes into actual keys
 */
KeyboardDriver::KeyboardDriver(KeyboardScanCodeSet& scs) : scan_code_set(scs), keyboard_data_port(0x60) {
}

KeyboardDriver::~KeyboardDriver() {
}

void KeyboardDriver::set_on_key_press(const KeyEvent &event) {
    on_key_press = event;
}

s16 KeyboardDriver::handled_interrupt_no() {
    return Interrupts::Keyboard;
}

CpuState* KeyboardDriver::on_interrupt(CpuState* cpu_state) {
    handle_keyboard_interrupt();
    return cpu_state;
}

void KeyboardDriver::handle_keyboard_interrupt() {
    u8 key_code = keyboard_data_port.read();
    Key key = scan_code_set.push_code(key_code);
    if (key != Key::INVALID)
        on_key_press(key);
}

} /* namespace drivers */
