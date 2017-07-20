/**
 *   @file: AtaDriver.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_ATADRIVER_H_
#define SRC_DRIVERS_ATADRIVER_H_

#include "DeviceDriver.h"

namespace drivers {

/**
 * @brief   AtaDevice represents any of 4 ata hdd devices: primary-master, primary-slave, secondary-master, secondary-slave.
 *          Both primary ata bus devices use int 14 and both secondary ata bus devices use int 15,
 *          so they are grouped into AtaPrimaryBusDriver and AtaSecondaryBusDriver
 */
class AtaDevice {
public:
    AtaDevice(u16 port_base, bool is_master);
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state);

    bool is_present();
    bool read28(u32 sector, void* data, u32 count);
    bool write28(u32 sector, void const* data, u32 count);
    bool flush_cache();

    static const u16 BYTES_PER_SECTOR          {512};
    static const u16 PRIMARY_BUS_PORT_BASE     {0x1F0};     // interrupt_no 14
    static const u16 SECONDARY_BUS_PORT_BASE   {0x170};     // interrupt_no 15

private:
    u8 poll_ata_device();

    Port16bit   data_port;
    Port8bit    error_port;
    Port8bit    sector_count_port;
    Port8bit    lba_lo_port;
    Port8bit    lba_mi_port;
    Port8bit    lba_hi_port;
    Port8bit    device_port;
    Port8bit    cmd_status_port;
    Port8bit    control_port;
    bool        is_master;  // master/slave drive

    const u8    CMD_IDENTIFY        = 0xEC;
    const u8    CMD_READ_SECTORS    = 0x20;
    const u8    CMD_WRITE_SECTORS   = 0x30;
    const u8    CMD_FLUSH_CACHE     = 0xE7;
    const u8    STATUS_NOT_READY    = 0x80;
    const u8    STATUS_ERROR        = 0x01;
};

class AtaPrimaryBusDriver : public DeviceDriver {
public:
    AtaPrimaryBusDriver();
    static s16 handled_interrupt_no();
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) override;
    AtaDevice master_hdd;
    AtaDevice slave_hdd;
};

class AtaSecondaryBusDriver : public DeviceDriver {
public:
    AtaSecondaryBusDriver();
    static s16 handled_interrupt_no();
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) override;
    AtaDevice master_hdd;
    AtaDevice slave_hdd;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_ATADRIVER_H_ */
