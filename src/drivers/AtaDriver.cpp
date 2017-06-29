/**
 *   @file: AtaDriver.cpp
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#include "AtaDriver.h"
#include "ScreenPrinter.h"

namespace drivers {

AtaDriver::AtaDriver(u16 port_base, bool is_master) :
        is_master(is_master),
        data_port(port_base),
        error_port(port_base + 1),
        sector_count_port(port_base + 2),
        lba_lo_port(port_base + 3),
        lba_mi_port(port_base + 4),
        lba_hi_port(port_base + 5),
        device_port(port_base + 6),
        cmd_port(port_base + 7),
        control_port(port_base + 0x206) {
}

void AtaDriver::identify() {
    ScreenPrinter& printer = ScreenPrinter::instance();

    device_port.write(is_master ? 0xA0 : 0xB0);
    control_port.write(0);
    device_port.write(0xA0);
    u8 status = cmd_port.read();
    if (status == 0xFF) {
        printer.format("[NONE]");
        return; // no device
    }

    device_port.write(is_master ? 0xA0 : 0xB0);
    sector_count_port.write(0);
    lba_lo_port.write(0);
    lba_mi_port.write(0);
    lba_hi_port.write(0);
    cmd_port.write(0xEC); // identify yourself

    status = cmd_port.read();
    if (status == 0x00) {
        printer.format("[NONE]");
        return; // no device
    }

    while (((status & 0x80) == 0x80) && // device not ready
           ((status & 0x01) != 0x01))   // error occured
        status = cmd_port.read();

    if (status & 0x01) {
        printer.format("ATA ERROR");
        return;
    }

    // identification data ready to read
    for (u16 i = 0; i < 256; i++) {
        u16 data = data_port.read();
        char s[3] {"xx"};
        s[0] = (data & 0xFF);
        s[1] = (data >> 8) & 0xFF;
        printer.format("%", s);
    }
}

void AtaDriver::read28(u32 sector, void* data, u32 count) {
    ScreenPrinter& printer = ScreenPrinter::instance();
    u8* dst = (u8*)data;

    if (sector >= (1 << 28)) {
        printer.format("Cant write to sector that far: %", sector);
        return;
    }

    if (count > 512) {
        printer.format("Cant write across 512 bytes sectors: sector %, count %", sector, count);
        return;
    }

    // select the device
    device_port.write((is_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);

    lba_lo_port.write(sector & 0x000000FF);
    lba_mi_port.write((sector & 0x0000FF00) >> 8);
    lba_hi_port.write((sector & 0x00FF0000) >> 16);
    cmd_port.write(0x20); // write

    // wait for the data to be ready
    u8 status = cmd_port.read();
    while (((status & 0x80) == 0x80) && // device not ready
           ((status & 0x01) != 0x01))   // error occured
        status = cmd_port.read();

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
}

void AtaDriver::write28(u32 sector, u8 const * data, u32 count) {
    ScreenPrinter& printer = ScreenPrinter::instance();

    if (sector >= (1 << 28)) {
        printer.format("Cant write to sector that far: %", sector);
        return;
    }

    if (count > BYTES_PER_SECTOR) {
        printer.format("Cant write across % bytes sectors: sector %, count %", BYTES_PER_SECTOR, sector, count);
        return;
    }

    // select the device
    device_port.write((is_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);

    lba_lo_port.write(sector & 0x000000FF);
    lba_mi_port.write((sector & 0x0000FF00) >> 8);
    lba_hi_port.write((sector & 0x00FF0000) >> 16);
    cmd_port.write(0x30); // write

    // write data to sector
    for (u16 i = 0; i < count; i += 2) {
        u16 data_chunk = data[i];
        if (i + 1 < count)
            data_chunk |= data[i+1] << 8;

        data_port.write(data_chunk);
    }

    // fill up rest of the sector with blanks, we always need to write entire sector
    for (u16 i = count + (count % 2); i < BYTES_PER_SECTOR; i += 2)
        data_port.write(0x0000);
}

void AtaDriver::flush_cache() {
    ScreenPrinter& printer = ScreenPrinter::instance();

    device_port.write(is_master ? 0xE0 : 0xF0);
    cmd_port.write(0xE7);

    u8 status = cmd_port.read();
    if (status == 0x00)
        return;

    // wait till cache is flushed
    while (((status & 0x80) == 0x80) &&
           ((status & 0x01) != 0x01))
        status = cmd_port.read();

    if (status & 0x01)
    {
        printer.format("ATA flush cache ERROR");
        return;
    }
}


} /* namespace drivers */
