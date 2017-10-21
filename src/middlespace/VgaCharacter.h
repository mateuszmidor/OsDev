/*
 * VgaCharacter.h
 *
 *  Created on: Oct 20, 2017
 *      Author: mateusz
 */

#ifndef SRC_MIDDLESPACE_VGACHARACTER_H_
#define SRC_MIDDLESPACE_VGACHARACTER_H_

namespace drivers {

// ega 16 color pallete index, mode that GRUB lefts us in
enum EgaColor : u8 {
    Black      = 0,
    Blue       = 1,
    Green      = 2,
    Cyan       = 3,
    Red        = 4,
    Magenta    = 5,
    Brown      = 6,
    LightGray  = 7,
    DarkGray   = 8,
    LightBlue  = 9,
    LightGreen = 10,
    LightCyan  = 11,
    LightRed   = 12,
    Pink       = 13,
    Yellow     = 14,
    White      = 15
};

/**
 * This is the structure describing single character in VGA buffer
 */
struct VgaCharacter {
    VgaCharacter(u16 c) {
        character = c & 0xFF;
        fg_color = (c >> 8) & 0xF;
        bg_color = (c >> 12) & 0xF;
    }

    VgaCharacter(u8 c, u8 fg, u8 bg) {
        character = c;
        fg_color = fg;
        bg_color = bg;
    }

    operator u16() {
        return *((u16*)this);
    }

    s8      character;
    u8      fg_color    : 4;
    u8      bg_color    : 4;
};

}; // namespace drivers

#endif /* SRC_MIDDLESPACE_VGACHARACTER_H_ */
