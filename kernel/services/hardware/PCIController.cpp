/**
 *   @file: PCIController.cpp
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

//#include "KernelLog.h"
#include "StringUtils.h"
#include "PCIController.h"
//#include "VgaDriver.h"

//using utils::KernelLog;
using namespace kstd;
namespace hardware {

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

string PCIController::drivers_to_string() {
    string out;

    for (u8 bus = 0; bus < 8; bus++)
        for (u8 device = 0; device < 32; device++) {
            u8 num_functions = device_has_functions(bus, device) ? 8 : 1;
            for (u8 function = 0; function < num_functions; function++) {
                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF) // this means no function
                    continue;

                for (u8 bar_num = 0; bar_num < 6; bar_num++) {
                    BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
                    if (bar.address && (bar.type == BaseAddressRegisterType::InputOutput))
                        dev.port_base = (u32)(u64)bar.address; // for IO registers, address is port number
                }

                out += StringUtils::format("PCI BUS %, DEVICE %, FUNCTION %", bus, device, function);
                out += StringUtils::format(" = VENDOR_ID % %, ", (dev.vendor_id & 0xFF00) >> 8, dev.vendor_id & 0xFF);
                out += StringUtils::format("DEVICE_NO % % ", (dev.device_id & 0xFF00) >> 8, dev.device_id & 0xFF);
                /* DeviceDriver drv = */ //get_driver(dev);
                out += StringUtils::format("\n");
            }
        }

    return out;
}

//void PCIController::enumerate_device_drivers(OnDeviceDriver on_driver) {
//    for (u8 bus = 0; bus < 8; bus++)
//        for (u8 device = 0; device < 32; device++) {
//            u8 num_functions = device_has_functions(bus, device) ? 8 : 1;
//            for (u8 function = 0; function < num_functions; function++) {
//                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, function);
//
//                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF) // this means no function
//                    continue;
//
//                for (u8 bar_num = 0; bar_num < 6; bar_num++) {
//                    BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
//                    if (bar.address && (bar.type == BaseAddressRegisterType::InputOutput))
//                        dev.port_base = (u32)(u64)bar.address; // for IO registers, address is port number
//                }
//
//                if (auto drv = get_driver(dev))
//                    on_driver(drv);
//            }
//        }
//}

//void PCIController::install_drivers_into(drivers::DriverManager& driver_manager) {
//    for (u8 bus = 0; bus < 8; bus++)
//        for (u8 device = 0; device < 32; device++) {
//            u8 num_functions = device_has_functions(bus, device) ? 8 : 1;
//            for (u8 function = 0; function < num_functions; function++) {
//                PCIDeviceDescriptor dev = get_device_descriptor(bus, device, function);
//
//                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF) // this means no function
//                    continue;
//
//                for (u8 bar_num = 0; bar_num < 6; bar_num++) {
//                    BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
//                    if (bar.address && (bar.type == BaseAddressRegisterType::InputOutput))
//                        dev.port_base = (u32)(u64)bar.address; // for IO registers, address is port number
//                }
//
//                get_driver(dev, driver_manager);
//
//            }
//        }
//}

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

BaseAddressRegister PCIController::get_base_address_register(u16 bus, u16 device, u16 function, u16 bar_no) {
    BaseAddressRegister result;

    u32 header_type = read(bus, device, function, 0x0E) & 0x7F;
    u8 max_bars = 6 - (4 * header_type);
    if (bar_no > max_bars)
        return result;

    u32 bar_value = read(bus, device, function, 0x10 + 4 * bar_no);
    result.type = (bar_value & 0x01) ? BaseAddressRegisterType::InputOutput : BaseAddressRegisterType::MemoryMapping;
    u32 temp;

    if (result.type == BaseAddressRegisterType::MemoryMapping) {
        switch ((bar_value >> 1) & 0x03) {
        case 0: break; // 32 bit mode
        case 1: break; // 20 bit mode
        case 2: break; // 64 bit mode
        }
        result.prefetchable = ((bar_value >> 3) & 0x01) == 0x01;
    } else {
        result.address = (u8*)(u64)(bar_value & ~0x03);
        result.prefetchable = false;
    }


    return result;
}

//DeviceDriverPtr PCIController::get_driver(PCIDeviceDescriptor &dev, drivers::DriverManager& driver_manager) {
//    KernelLog& klog = KernelLog::instance();
//
//    switch (dev.vendor_id) {
//    case 0x1022:    // AMD
//        switch (dev.device_id) {
//        case 0x2000:    // am79c973 network chip
//            //klog.format("[AMD am79c973]");
//            break;
//        }
//        break;
//
//    case 0x8086:    // intel
//        break;
//    }
//
//    switch (dev.class_id) { // generic devices
//    case 0x03:  // graphics
//        switch (dev.subclass_id) {
//        case 0x00:  // vga
//            //klog.format("[Generic VGA]");
////            driver_manager.install_driver(std::make_shared<drivers::VgaDriver>());
//            break;
//        }
//    }
//    return {};
//}

u32 PCIController::make_id(u16 bus, u16 device, u16 function, u32 register_offset) {
    return (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | ((register_offset & 0xFC));
}

} // namespace hardware {
