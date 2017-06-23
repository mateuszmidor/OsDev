/**
 *   @file: PCIController.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#include "PCIController.h"

PCIController::PCIController() {
}

PCIController::~PCIController() {
}

u32 PCIController::read(u16 bus, u16 device, u16 function, u32 register_offset) {
    u32 id = make_id(bus, device, function, register_offset);
    cmd_port.write(id);
    u32 result = data_port.read();
    return result >> (8 * (register_offset % 4));
}

void PCIController::write(u16 bus, u16 device, u16 function, u32 register_offset, u32 value) {
    u32 id = make_id(bus, device, function, register_offset);
    cmd_port.write(id);
    data_port.write(value);
}

bool PCIController::device_has_functions(u16 bus, u16 device) {
    return read(bus, device, 0, 0x0E) & (1 << 7);
}

#include "ScreenPrinter.h"

void PCIController::select_drivers() {
    ScreenPrinter &printer = ScreenPrinter::instance();
    for (u8 bus = 0; bus < 8; bus++)
        for (u8 device = 0; device < 32; device++) {
            u8 num_functions = device_has_functions(bus, device) ? 8 : 1;
            for (u8 function = 0; function < num_functions; function++) {
                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF) // this means end of functions for this device
                    break;

                printer.format("PCI BUS %, DEVICE %, FUNCTION %", bus, device, function);
                printer.format(" = VENDOR_ID % %, ", (dev.vendor_id & 0xFF00) >> 8, dev.vendor_id & 0xFF);
                printer.format("DEVICE_NO % %\n", (dev.device_id & 0xFF00) >> 8, dev.device_id & 0xFF);
            }
        }
}

PCIDeviceDescriptor PCIController::get_device_descriptor(u16 bus, u16 device, u16 function) {
    PCIDeviceDescriptor result;
    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = read(bus, device, function, 0x00);
    result.device_id = read(bus, device, function, 0x02);

    result.class_id = read(bus, device, function, 0x0B);
    result.subclass_id = read(bus, device, function, 0x0A);
    result.interface_id = read(bus, device, function, 0x09);

    result.revision = read(bus, device, function, 0x08);
    result.interrupt_no = read(bus, device, function, 0x3C);

    return result;
}

u32 PCIController::make_id(u16 bus, u16 device, u16 function, u32 register_offset) {
    return (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | ((register_offset & 0xFC));
}
