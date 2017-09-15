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

struct CpuMultimediaExtensions {
    bool sse    = false;
    bool sse2   = false;
    bool sse3   = false;
    bool ssse3  = false;
    bool sse4_1 = false;
    bool sse4_2 = false;
    bool sse4a  = false;
    bool sse5   = false;
    bool avx    = false;

    kstd::string to_string() const;
};

class CpuInfo {
public:
    u32 get_peak_mhz() const;
    kstd::string get_vendor() const;
    CpuMultimediaExtensions get_multimedia_extensions() const;
    void print_to_klog() const;

private:
    u64 rtdsc() const;
    void __cpuid(int* cpuinfo, int info) const;
    unsigned long long _xgetbv(unsigned int index) const;
};

} // namespace utils
#endif /* SRC_CPUINFO_H_ */
