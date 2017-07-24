/**
 *   @file: PCIController.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_PCICONTROLLER_H_
#define SRC_PCICONTROLLER_H_

#include <memory>
#include <functional>
#include "Port.h"
#include "DeviceDriver.h"
#include "DriverManager.h"

enum BaseAddressRegisterType {
    MemoryMapping   = 0,
    InputOutput     = 1
};

struct BaseAddressRegister {
    bool prefetchable;
    u8 *address;
    u32 size;
    BaseAddressRegisterType type;
};

struct PCIDeviceDescriptor {
    u32 port_base;
    u32 interrupt_no;

    u16 bus;
    u16 device;
    u16 function;

    u16 vendor_id;
    u16 device_id;

    u8 class_id;
    u8 subclass_id;
    u8 interface_id;

    u8 revision;
};

using DeviceDriverPtr = std::shared_ptr<drivers::DeviceDriver>;
using OnDeviceDriver = std::function<void(DeviceDriverPtr)>;

class PCIController {
public:
    u32 read(u16 bus, u16 device, u16 function, u32 register_offset);
    void write(u16 bus, u16 device, u16 function, u32 register_offset, u32 value);
    bool device_has_functions(u16 bus, u16 device);
    void drivers_to_klog();
    void enumerate_device_drivers(OnDeviceDriver on_driver);
    void install_drivers_into(drivers::DriverManager& driver_manager);
    PCIDeviceDescriptor get_device_descriptor(u16 bus, u16 device, u16 function);
    BaseAddressRegister get_base_address_register(u16 bus, u16 device, u16 function, u16 bar_no);
    DeviceDriverPtr get_driver(PCIDeviceDescriptor &dev, drivers::DriverManager& driver_manager);

private:
    Port32bit data_port { 0xCFC };
    Port32bit cmd_port { 0xCF8 };

    u32 make_id(u16 bus, u16 device, u16 function, u32 register_offset);
};

#endif /* SRC_PCICONTROLLER_H_ */
