/**
 *   @file: Port.h
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_PORT_H_
#define SRC_PORT_H_

#include "types.h"

namespace hardware {

/**
 * @class   Port
 * @brief   Just a base class for Port8bit, Port8bitSlow, Port16bit and Port32bit
 */
class Port {
protected:
    Port(u16 port_number);
    u16 port_number;
};

class Port8bit : protected Port {
public:
    Port8bit(u16 port_number);
    void write(u8 data) const;
    u8 read() const;
};

class Port8bitSlow : public Port8bit {
public:
    Port8bitSlow(u16 port_number);
    void write(u8 data) const;
};

class Port16bit : protected Port {
public:
    Port16bit(u16 port_number);
    void write(u16 data) const;
    u16 read() const;
};

class Port32bit : protected Port {
public:
    Port32bit(u32 port_number);
    void write(u32 data) const;
    u32 read() const;
};

} // namespace hardware
#endif /* SRC_PORT_H_ */
