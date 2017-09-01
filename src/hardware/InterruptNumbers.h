/**
 *   @file: InterruptNumbers.h
 *
 *   @date: Jul 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_INTERRUPTNUMBERS_H_
#define SRC_INTERRUPTNUMBERS_H_

#include "types.h"

namespace hardware {

enum Interrupts : u16 {
    EXC_BASE        = 0x00,             // cpu exceptions start here
    PageFault       = EXC_BASE + 14,
    TaskExit        = EXC_BASE + 15,    // exception 15 is reserved, but we temporarily use it for signaling task exit
    EXC_MAX         = 0x20,             // cpu exception count

    IRQ_BASE        = 0x20,             // hw interrupts start here; IRQ_BASE is defined in interrupts.S
    Timer           = IRQ_BASE + 0,
    Keyboard        = IRQ_BASE + 1,
    Mouse           = IRQ_BASE + 12,
    PrimaryAta      = IRQ_BASE + 14,
    SecondaryAta    = IRQ_BASE + 15,

    SysCall         = 0x80,             // IRQ_BASE not considered here as int 0x80 is not being called by hw but by the user sofware
    Vga             = 0xFF,             // fake interrupt no; Vga sends no interrupts but it is needed to fit in InterruptManager interface
    IRQ_MAX         = 0x100
};

} // namespace hardware


#endif /* SRC_INTERRUPTNUMBERS_H_ */
