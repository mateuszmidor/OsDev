/**
 *   @file: MouseState.h
 *
 *   @date: Dec 5, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_MIDDLESPACE_MOUSESTATE_H_
#define KERNEL_MIDDLESPACE_MOUSESTATE_H_

#include "types.h"

namespace middlespace {

enum MouseButton : u8 {
    LEFT    = 0,
    RIGHT   = 1,
    MIDDLE  = 2
};

struct MouseState {
    s8      dx;             // X-Axis movement
    s8      dy;             // Y-Axis movement
    bool    buttons[3];     // see: MouseButton::LEFT, MouseButton::MIDDLE, MouseButton::RIGHT
};;

} /* namespace middlespace */

#endif /* KERNEL_MIDDLESPACE_MOUSESTATE_H_ */
