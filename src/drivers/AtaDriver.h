/**
 *   @file: AtaDriver.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_ATADRIVER_H_
#define SRC_DRIVERS_ATADRIVER_H_

#include "Port.h"

namespace drivers {

class AtaDriver {
public:
    AtaDriver(u16 port_base, bool is_master);
    void identify();
    void read28(u32 sector, u8* data, u32 count);
    void write28(u32 sector, u8 const * data, u32 count);
    void flush_cache();

    static constexpr u16 BYTES_PER_SECTOR  {512};
    static constexpr u16 FIRST_PORT_BASE  {0x1F0};     // interrupt_no 14
    static constexpr u16 SECOND_PORT_BASE   {0x170};     // interrupt_no 15
    static constexpr u16 THIRD_PORT_BASE   {0x1E8};
    static constexpr u16 FOURTH_PORT_BASE  {0x168};

private:
    Port16bit   data_port;
    Port8bit    error_port;
    Port8bit    sector_count_port;
    Port8bit    lba_lo_port;
    Port8bit    lba_mi_port;
    Port8bit    lba_hi_port;
    Port8bit    device_port;
    Port8bit    cmd_port;
    Port8bit    control_port;
    bool        is_master;  // master/slave drive
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_ATADRIVER_H_ */
