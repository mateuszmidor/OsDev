/**
 *   @file: MouseDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_MOUSEDRIVER_H_
#define SRC_DRIVERS_MOUSEDRIVER_H_

#include "DeviceDriver.h"

namespace drivers {

class MouseDriver : public DeviceDriver {
public:
    MouseDriver();
    virtual ~MouseDriver();

    void on_interrupt() override;
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_MOUSEDRIVER_H_ */
