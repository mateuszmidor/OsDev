/**
 *   @file: CpuInfo.h
 *
 *   @date: Jun 7, 2017
 * @author: mateusz
 */


#ifndef SRC_CPUINFO_H_
#define SRC_CPUINFO_H_

#include "kstd.h"

namespace utils {

class CpuInfo {
public:
    u32 get_peak_mhz() const;
    kstd::string get_vendor() const;
    void print_to_klog() const;

private:
    u64 rtdsc() const;
};

} // namespace utils
#endif /* SRC_CPUINFO_H_ */
