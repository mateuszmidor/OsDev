/**
 *   @file: KeyboardDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "KeyboardDriver.h"

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
    return 33;
}

void KeyboardDriver::on_interrupt() {
    u8 key_code = keyboard_data_port.read();
    s8 ascii_key = scan_code_set.code_to_ascii(key_code).c_str()[0];
    on_key_press(ascii_key);
}

} /* namespace drivers */
