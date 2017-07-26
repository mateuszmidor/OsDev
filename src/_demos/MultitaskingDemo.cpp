/*
 * MultitaskingDemo.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: mateusz
 */

#include "MultitaskingDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "VgaDriver.h"

using namespace utils;
using namespace drivers;

namespace demos {

utils::LimitedAreaScreenPrinter MultitaskingDemo::printer(0, 0, 89, 29);

MultitaskingDemo::MultitaskingDemo()  {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();
    auto vga = driver_manager.get_driver<VgaDriver>();

    if (!vga) {
        klog.format("MultitaskingDemo::run: no VgaDriver\n");
        return;
    }
}

void MultitaskingDemo::run(u64 arg) {
    char buff[] = {(char)arg, '\0'};
    while (true) {
        printer.format("%", buff);
        asm("hlt");
    }
}
} /* namespace demos */
