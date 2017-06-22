/**
 *   @file: MouseDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_MOUSEDRIVER_H_
#define SRC_DRIVERS_MOUSEDRIVER_H_

#include <functional>
#include "DeviceDriver.h"

namespace drivers {

using MouseButtonEvent = std::function<void(u8 button)>;
using MouseMoveEvent = std::function<void(s8 dx, s8 dy)>;

class MouseDriver : public DeviceDriver {
public:
    MouseDriver();
    virtual ~MouseDriver();

    s16 handled_interrupt_no() override;
    void on_interrupt() override;

    void set_on_move(const MouseMoveEvent &event);
    void set_on_down(const MouseButtonEvent &event);
    void set_on_up(const MouseButtonEvent &event);

private:
    u8 buttons = 0;
    u8 offset = 0;
    u8 buffer[3];
    Port8bit mouse_cmd_port {0x64};
    Port8bit mouse_data_port {0x60};
    MouseMoveEvent on_move = [] (s8 dx, s8 dy) { /* do nothing */ };
    MouseButtonEvent on_down = [] (u8 button) { /* do nothing */ };
    MouseButtonEvent on_up = [] (u8 button) { /* do nothing */ };

    bool setup();
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_MOUSEDRIVER_H_ */
