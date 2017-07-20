/**
 *   @file: KeyboardDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDDRIVER_H_
#define SRC_DRIVERS_KEYBOARDDRIVER_H_

#include <functional>
#include <memory>
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
    cpu::CpuState* on_interrupt(cpu::CpuState* cpu_state) override;

private:
    Port8bit keyboard_data_port;
    KeyboardScanCodeSet& scan_code_set;
    KeyEvent on_key_press = [](u8) { /* do nothing */ };

    void handle_keyboard_interrupt();
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDDRIVER_H_ */
