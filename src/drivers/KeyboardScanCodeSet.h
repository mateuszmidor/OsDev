/**
 *   @file: ScanCodeSet1.h
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_DRIVERS_KEYBOARDSCANCODESET_H_
#define SRC_DRIVERS_KEYBOARDSCANCODESET_H_

#include <array>
#include "kstd.h"
#include "types.h"

namespace drivers {

class KeyboardScanCodeSet {
public:
    virtual ~KeyboardScanCodeSet() {}
    virtual const kstd::string& code_to_ascii(u8 key_code) const = 0;
};

class KeyboardScanCodeSet1 : public KeyboardScanCodeSet {
public:
    KeyboardScanCodeSet1();
    //kstd::string code_to_key_description(u8 key_code) const;
    const kstd::string& code_to_ascii(u8 key_code) const override;

private:
   // kstd::string key_description[256];
    std::array<kstd::string, 256> key_ascii;
};

}   /* namespace drivers */

#endif /* SRC_DRIVERS_KEYBOARDSCANCODESET_H_ */
