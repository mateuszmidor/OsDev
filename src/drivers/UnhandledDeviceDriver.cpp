/**
 *   @file: UnhandledDeviceDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "UnhandledDeviceDriver.h"

namespace drivers {


s16 UnhandledDeviceDriver::handled_interrupt_no() {
    return -1; // not relevant; can handle any interrupt ;)
}

cpu::CpuState* UnhandledDeviceDriver::on_interrupt(cpu::CpuState* cpu_state) {
    // print warning msg that UnhandledDeviceDriver interrupt has been raised
    // WARNING: unhandled interrupt may stop PIC from sending any further interrupts so this is error
    return cpu_state;
}

} /* namespace drivers */
