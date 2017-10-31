/**
 *   @file: ScanCodeSet1.h
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDSCANCODESET_H_
#define SRC_DRIVERS_KEYBOARDSCANCODESET_H_

#include <array>
#include "types.h"
#include "Key.h"

namespace middlespace {

class KeyboardScanCodeSet {
public:
    virtual ~KeyboardScanCodeSet() {}
    virtual const Key push_code(u8 key_code) = 0;
};

class KeyboardScanCodeSet1 : public KeyboardScanCodeSet {
public:
    KeyboardScanCodeSet1();
    const Key push_code(u8 key_code) override;

private:
    std::array<Key, 256> basic_code_key;
    std::array<Key, 256> basic_shift_code_key;
    std::array<Key, 256> extended_code_key;
    bool extended_key_incoming = false;
    bool is_lshift_down = false;
    bool is_caps_active = false;
    static const int EXTENDED_KEY_SEQUENCE_BEGIN = 0xE0;
};

}   /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDSCANCODESET_H_ */
