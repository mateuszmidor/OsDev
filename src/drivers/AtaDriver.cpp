/**
 *   @file: AtaDriver.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "AtaDriver.h"
#include "ScreenPrinter.h"

namespace drivers {

AtaDevice::AtaDevice(u16 port_base, bool is_master) :
        is_master(is_master),
        data_port(port_base),
        error_port(port_base + 1),
        sector_count_port(port_base + 2),
        lba_lo_port(port_base + 3),
        lba_mi_port(port_base + 4),
        lba_hi_port(port_base + 5),
        device_port(port_base + 6),
        cmd_status_port(port_base + 7),
        control_port(port_base + 0x206) {
}

cpu::CpuState* AtaDevice::on_interrupt(cpu::CpuState* cpu_state) {
    // TODO: implement irq driven operation instead of polling
    return cpu_state;
}

bool AtaDevice::is_present() {
//    ScreenPrinter& printer = ScreenPrinter::instance();

    device_port.write(is_master ? 0xA0 : 0xB0);
    sector_count_port.write(0);
    lba_lo_port.write(0);
    lba_mi_port.write(0);
    lba_hi_port.write(0);
    cmd_status_port.write(CMD_IDENTIFY); // identify yourself

    u8 status = poll_ata_device();
    if (status == 0x00)
        return false;

    if (status & STATUS_ERROR) {
//        printer.format("ATA ERROR");
        return false;
    }

    // identification data ready to read
    for (u16 i = 0; i < 256; i++) {
        u16 data = data_port.read();
        char s[3] {"xx"};
        s[0] = (data & 0xFF);
        s[1] = (data >> 8) & 0xFF;
//        printer.format("%", s);
    }

    return true;
}

bool AtaDevice::read28(u32 sector, void* data, u32 count) {
    
    ScreenPrinter& printer = ScreenPrinter::instance();
    u8* dst = (u8*)data;

    if (sector >= (1 << 28)) {
        printer.format("Cant read from sector that far: %\n", sector);
        return false;
    }

    if (count > 512) {
        printer.format("Cant write across 512 bytes sectors: sector %, count %\n", sector, count);
        return false;
    }

    // select the device
    device_port.write((is_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);

    lba_lo_port.write(sector & 0x000000FF);
    lba_mi_port.write((sector & 0x0000FF00) >> 8);
    lba_hi_port.write((sector & 0x00FF0000) >> 16);
    cmd_status_port.write(CMD_READ_SECTORS); // read

    // wait for the data to be ready
    u8 status = poll_ata_device();

    // read data from sector
    for (u16 i = 0; i < count; i += 2) {
        u16 data_chunk = data_port.read();
        dst[i] = data_chunk & 0xFF;
        if (i + 1 < count)
            dst[i + 1] = data_chunk >> 8;
    }

    // read rest of the sector, we always need to read entire sector
    for (u16 i = count + (count % 2); i < BYTES_PER_SECTOR; i += 2)
        data_port.read();

    return true;
}

bool AtaDevice::write28(u32 sector, void const* data, u32 count) {
    ScreenPrinter& printer = ScreenPrinter::instance();
    u8 const* dst = (u8*)data;

    if (sector >= (1 << 28)) {
        printer.format("Cant write to sector that far: %\n", sector);
        return false;
    }

    if (count > BYTES_PER_SECTOR) {
        printer.format("Cant write across % bytes sectors: sector %, count %\n", BYTES_PER_SECTOR, sector, count);
        return false;
    }

    // select the device
    device_port.write((is_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);

    lba_lo_port.write(sector & 0x000000FF);
    lba_mi_port.write((sector & 0x0000FF00) >> 8);
    lba_hi_port.write((sector & 0x00FF0000) >> 16);
    cmd_status_port.write(CMD_WRITE_SECTORS); // write

    // write data to sector
    for (u16 i = 0; i < count; i += 2) {
        u16 data_chunk = dst[i];
        if (i + 1 < count)
            data_chunk |= dst[i+1] << 8;

        data_port.write(data_chunk);
    }

    // fill up rest of the sector with blanks, we always need to write entire sector
    for (u16 i = count + (count % 2); i < BYTES_PER_SECTOR; i += 2)
        data_port.write(0x0000);


    return flush_cache();
}

bool AtaDevice::flush_cache() {
    ScreenPrinter& printer = ScreenPrinter::instance();

    device_port.write(is_master ? 0xE0 : 0xF0);
    cmd_status_port.write(CMD_FLUSH_CACHE);

    // wait till cache is flushed
    u8 status = poll_ata_device();
    if (status == 0x00)
        return false;

    if (status & STATUS_ERROR)
    {
        printer.format("ATA flush cache ERROR");
        return false;
    }

    return true;
}

/**
 * Instead of polling, interrupts should be used for better performance
 */
u8 AtaDevice::poll_ata_device() {
    u8 status = cmd_status_port.read();
    if (status == 0x00)
        return 0;

    while (((status & STATUS_NOT_READY) == STATUS_NOT_READY) &&
           ((status & STATUS_ERROR) != STATUS_ERROR))
        status = cmd_status_port.read();

    return status;
}



AtaPrimaryBusDriver::AtaPrimaryBusDriver() :
        master_hdd(AtaDevice::PRIMARY_BUS_PORT_BASE, true),
        slave_hdd(AtaDevice::PRIMARY_BUS_PORT_BASE, false) {
}

s16 AtaPrimaryBusDriver::handled_interrupt_no() {
    return 0x20 + 14; // IRQ_BASE + ...
}

cpu::CpuState* AtaPrimaryBusDriver::on_interrupt(cpu::CpuState* cpu_state) {
    master_hdd.on_interrupt(cpu_state);
    slave_hdd.on_interrupt(cpu_state);
    return cpu_state; // no task switching here
}

AtaSecondaryBusDriver::AtaSecondaryBusDriver() :
        master_hdd(AtaDevice::SECONDARY_BUS_PORT_BASE, true),
        slave_hdd(AtaDevice::SECONDARY_BUS_PORT_BASE, false) {
}

s16 AtaSecondaryBusDriver::handled_interrupt_no() {
    return 0x20 + 15; // IRQ_BASE + ...
}

cpu::CpuState* AtaSecondaryBusDriver::on_interrupt(cpu::CpuState* cpu_state) {
    master_hdd.on_interrupt(cpu_state);
    slave_hdd.on_interrupt(cpu_state);
    return cpu_state; // no task switching here
}
} /* namespace drivers */
