/**
 *   @file: PCIController.h
 *
 *   @date: Jun 23, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_PCICONTROLLER_H_
#define SRC_PCICONTROLLER_H_

#include "Port.h"


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

class PCIController {
public:
    PCIController();
    virtual ~PCIController();

    u32 read(u16 bus, u16 device, u16 function, u32 register_offset);
    void write(u16 bus, u16 device, u16 function, u32 register_offset, u32 value);
    bool device_has_functions(u16 bus, u16 device);
    void select_drivers();
    PCIDeviceDescriptor get_device_descriptor(u16 bus, u16 device, u16 function);

private:
    Port32bit data_port { 0xCFC };
    Port32bit cmd_port { 0xCF8 };

    u32 make_id(u16 bus, u16 device, u16 function, u32 register_offset);
};

#endif /* SRC_PCICONTROLLER_H_ */
