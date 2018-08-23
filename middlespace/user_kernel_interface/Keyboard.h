/**
 *   @file: Keyboard.h
 *
 *   @date: Oct 30, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MIDDLESPACE_KEYBOARD_H_
#define SRC_MIDDLESPACE_KEYBOARD_H_

#include "types.h"

namespace middlespace {

/**
 * @brief   Key represents any keyboard key.
 *          Function keys are bit-orable with printable keys, so we can encode eg. CTRL+C
 */
enum Key : u16 {
    // printable keys
    INVALID = 0,

    // function keys
    Esc         = 1 << 8,
    Tab         = 2 << 8,
    Enter       = 3 << 8,
    Backspace   = 4 << 8,
    PgUp        = 5 << 8,
    PgDown      = 6 << 8,
    Up          = 7 << 8,
    Down        = 8 << 8,
    Left        = 9 << 8,
    Right       = 10 << 8,
    Home        = 11 << 8,
    End         = 12 << 8,
    CapsLock    = 13 << 8,
    F1          = 14 << 8,
    F2          = 15 << 8,
    F3          = 16 << 8,
    F4          = 17 << 8,
    F5          = 18 << 8,
    F6          = 19 << 8,
    F7          = 20 << 8,
    F8          = 21 << 8,
    F9          = 22 << 8,
    F10         = 23 << 8,
    F11         = 24 << 8,
    F12         = 25 << 8,
    LCtrl       = 26 << 8,
    LCtrl_Released  = 27 << 8,
    LAlt            = 28 << 8,
    LAlt_Released   = 29 << 8,
    LShift          = 30 << 8,
    LShift_Released = 31 << 8,

    // this is mask to dig out function keys: is_functional = key & Key::FUNCTIONAL
    FUNCTIONAL = 0xFF00
};

namespace keyboard {

/**
 * @return  Printable(ASCII) part of the "key"
 */
static Key get_printable(Key key) {
    return (Key)(key & ~Key::FUNCTIONAL);
}

/**
 * @return  Functional(enter, esc, ctrl, F1, etc) part of the "key"
 */
static Key get_functional(Key key) {
    return (Key)(key & Key::FUNCTIONAL);
}
}

};

#endif /* SRC_MIDDLESPACE_KEYBOARD_H_ */
