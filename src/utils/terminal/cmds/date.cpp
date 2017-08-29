/**
 *   @file: date.cpp
 *
 *   @date: Aug 29, 2017
 * @author: Mateusz Midor
 */

#include "date.h"

namespace cmds {

void date::run() {
    u16 year    = read_byte(0x9);
    u16 month   = read_byte(0x8);
    u16 day     = read_byte(0x7);
    u16 hour    = read_byte(0x4);
    u16 minute  = read_byte(0x2);
    u16 second  = read_byte(0x0);

    env->printer->format("%-%-% %:%:% UTC\n", bin(year) + 2000, bin(month), bin(day), bin(hour), bin(minute), bin(second));
}

u8 date::read_byte(u8 offset) const {
    address.write(offset);
    return data.read();
}

// convert bcd to bin
u8 date::bin(u8 bcd) const {
    return (bcd / 16) * 10 + (bcd & 0xF);
}

} /* namespace cmds */
