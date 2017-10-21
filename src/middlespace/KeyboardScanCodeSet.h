/**
 *   @file: ScanCodeSet1.h
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDSCANCODESET_H_
#define SRC_DRIVERS_KEYBOARDSCANCODESET_H_

#include <array>
#include "types.h"

namespace drivers {

/**
 * @brief   Key represents any keyboard key.
 *          If it is printable key, (key & 0xFF00) = 0
 *          If it is function key, (key & 0xFF00) != 0
 */
enum Key : u16 {
    // printable keys
    INVALID = 0,


    // function keys
    Esc     = 256,
    Tab,
    Enter,
    Backspace,
    PgUp, PgDown,
    Up, Down, Left, Right,
    Home, End,
    LShift, LShift_Released,
    CapsLock,
    LCtrl, LCtrl_Released,
    LAlt, LAlt_Released,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    FUNCTIONAL = 0xFF00 // this is mask to dig out function keys
};

class KeyboardScanCodeSet {
public:
    virtual ~KeyboardScanCodeSet() {}
    virtual const Key push_code(u8 key_code) = 0;
};

class KeyboardScanCodeSet1 : public KeyboardScanCodeSet {
public:
    KeyboardScanCodeSet1();
    const Key push_code(u8 key_code) override;

private:
    std::array<Key, 256> basic_code_key;
    std::array<Key, 256> basic_shift_code_key;
    std::array<Key, 256> extended_code_key;
    bool extended_key_incoming = false;
    bool is_lshift_down = false;
    bool is_caps_active = false;
    static const int EXTENDED_KEY_SEQUENCE_BEGIN = 0xE0;
};

}   /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDSCANCODESET_H_ */
