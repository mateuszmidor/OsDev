/**
 *   @file: CpuInfo.h
 *
 *   @date: Jun 7, 2017
 * @author: mateusz
 */

#include "ScreenPrinter.h"

#ifndef SRC_CPUINFO_H_
#define SRC_CPUINFO_H_

class CpuInfo {
public:
    CpuInfo();
    void print(ScreenPrinter &p);

private:
    char vendor[17];
    void getCpuVendor(char buff[17]);
};

#endif /* SRC_CPUINFO_H_ */
