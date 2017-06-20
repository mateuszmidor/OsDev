/**
 *   @file: ScanCodeSet1.h
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_SCANCODESET1_H_
#define SRC_SCANCODESET1_H_

#include "kstd.h"
#include "types.h"


class ScanCodeSet1 {
public:
    ScanCodeSet1();
    kstd::string code_to_key_description(u8 key_code) const;
    kstd::string code_to_ascii(u8 key_code) const;

private:
    kstd::string key_description[256];
    kstd::string key_ascii[256];
};

#endif /* SRC_SCANCODESET1_H_ */
