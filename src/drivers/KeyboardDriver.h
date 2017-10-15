/**
 *   @file: KeyboardDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDDRIVER_H_
#define SRC_DRIVERS_KEYBOARDDRIVER_H_

#include <functional>
#include "DeviceDriver.h"
#include "KeyboardScanCodeSet.h"

namespace drivers {

using KeyEvent = std::function<void(Key)>;

class KeyboardDriver : public DeviceDriver {
public:
    KeyboardDriver(KeyboardScanCodeSet& scs);
    virtual ~KeyboardDriver();
    void set_on_key_press(const KeyEvent &event);

    static s16 handled_interrupt_no();
    hardware::CpuState* on_interrupt(hardware::CpuState* cpu_state) override;

private:
    hardware::Port8bit keyboard_data_port;
    KeyboardScanCodeSet& scan_code_set;
    KeyEvent on_key_press = [](u8) { /* do nothing */ };

    void handle_keyboard_interrupt();
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDDRIVER_H_ */
