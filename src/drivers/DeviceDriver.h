/**
 *   @file: Driver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_DEVICEDRIVER_H_
#define SRC_DRIVERS_DEVICEDRIVER_H_

#include "types.h"
#include "Port.h"

namespace drivers {

class DeviceDriver {
public:
    DeviceDriver();
    virtual ~DeviceDriver();

    virtual void on_interrupt() = 0;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_DEVICEDRIVER_H_ */
