/**
 *   @file: Port.cpp
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#include "Port.h"

namespace hardware {

Port::Port(u16 port_number) : port_number(port_number) {
}



Port8bit::Port8bit(u16 port_number) : Port(port_number) {
}

void Port8bit::write(u8 data) const {
    __asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (port_number));
}

u8 Port8bit::read() const {
    u8 result;
    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (port_number));
    return result;
}



Port8bitSlow::Port8bitSlow(u16 port_number) : Port8bit(port_number) {
}

void Port8bitSlow::write(u8 data) const {
    __asm__ volatile(
            "outb %0, %1\njmp 1f\n1: jmp 1f\n1:"
            : : "a" (data), "Nd" (port_number)
            );
}



Port16bit::Port16bit(u16 port_number) : Port(port_number) {
}

void Port16bit::write(u16 data) const {
    __asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (port_number));
}

u16 Port16bit::read() const {
    u16 result;
    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (port_number));
    return result;
}



Port32bit::Port32bit(u32 port_number) : Port(port_number) {
}

void Port32bit::write(u32 data) const {
    __asm__ volatile("outl %0, %1" : : "a" (data), "Nd" (port_number));
}

u32 Port32bit::read() const {
    u32 result;
    __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (port_number));
    return result;
}

} // namespace hardware
