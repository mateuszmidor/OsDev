/**
 *   @file: date.h
 *
 *   @date: Aug 29, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_CMDS_DATE_H_
#define SRC_UTILS_TERMINAL_CMDS_DATE_H_

#include "Port.h"
#include "CmdBase.h"

namespace cmds {

/**
 * Read date/time from RTC
 * See: https://www.codeproject.com/Articles/49813/Beginning-Operating-System-Development-Part-Four
 */
class date: public CmdBase {
public:
    date(u64 arg) : CmdBase(arg) {}
    void run() override;

private:
    u8 read_byte(u8 offset) const;
    u8 bin(u8 bcd) const;
    hardware::Port8bitSlow address  { 0x70 };
    hardware::Port8bitSlow data     { 0x71 };
};

} /* namespace cmds */

#endif /* SRC_UTILS_TERMINAL_CMDS_DATE_H_ */
