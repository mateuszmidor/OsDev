/**
 *   @file: Key.h
 *
 *   @date: Oct 30, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MIDDLESPACE_KEY_H_
#define SRC_MIDDLESPACE_KEY_H_

#include "types.h"

namespace middlespace {

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
};

#endif /* SRC_MIDDLESPACE_KEY_H_ */
