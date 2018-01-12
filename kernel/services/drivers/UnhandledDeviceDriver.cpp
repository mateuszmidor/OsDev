/**
 *   @file: UnhandledDeviceDriver.cpp
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#include "UnhandledDeviceDriver.h"
#include "KernelLog.h"

using logging::KernelLog;

namespace drivers {

/**
 * Constructor.
 * This is generic handler so you can set the interrupt_no that it handles so it knows what it handles :)
 */
UnhandledDeviceDriver::UnhandledDeviceDriver(u8 interrupt_no) : interrupt_no(interrupt_no) {
}

s16 UnhandledDeviceDriver::handled_interrupt_no() {
    return hardware::Interrupts::IRQ_MAX; // invalid interrupt no
}

hardware::CpuState* UnhandledDeviceDriver::on_interrupt(hardware::CpuState* cpu_state) {
    KernelLog& klog = KernelLog::instance();
    // print warning msg that UnhandledDeviceDriver interrupt has been raised
    // WARNING: unhandled interrupt may stop PIC from sending any further interrupts so this is error
    klog.format("\nUNHANDLED INTERRUPT %\n", interrupt_no);
    return cpu_state;
}

} /* namespace drivers */
