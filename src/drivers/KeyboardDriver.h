/**
 *   @file: KeyboardDriver.h
 *
 *   @date: Jun 22, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDDRIVER_H_
#define SRC_DRIVERS_KEYBOARDDRIVER_H_

#include "DeviceDriver.h"
#include "KeyboardScanCodeSet.h"
#include "VfsManager.h"

namespace drivers {

using KeyEvent = std::function<void(Key)>;

/**
 * @@brief  This class is a keyboard driver. On key stroke it writes Key into /dev/keyboard RAM file and runs on_key_press callback
 */
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
    KeyEvent on_key_press = [](u8) {    /* do nothing */ };
    filesystem::VfsEntryPtr keyboard;   /* most recent key is written here */

    void handle_keyboard_interrupt();
};

} /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDDRIVER_H_ */
