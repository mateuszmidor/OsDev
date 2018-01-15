/**
 *   @file: VgaPrinter.h
 *
 *   @date: Jan 12, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_UTILS_VGAPRINTER_H_
#define KERNEL_SERVICES_UTILS_VGAPRINTER_H_

#include "VgaDriver.h"
#include "Vga.h"

namespace utils {

class VgaPrinter {
public:
    VgaPrinter(drivers::VgaDriver& vga);
    void print(const char s[], middlespace::EgaColor c = middlespace::EgaColor::Yellow);
    void println(const char s[], middlespace::EgaColor c = middlespace::EgaColor::Yellow);

private:
    u32 current_row {0};
    u32 current_col {0};
    drivers::VgaDriver& vga;
};

} /* namespace utils */

#endif /* KERNEL_SERVICES_UTILS_VGAPRINTER_H_ */
