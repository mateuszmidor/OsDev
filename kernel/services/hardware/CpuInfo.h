/**
 *   @file: CpuInfo.h
 *
 *   @date: Jun 7, 2017
 * @author: Mateusz Midor
 */


#ifndef SRC_CPUINFO_H_
#define SRC_CPUINFO_H_

#include "types.h"
#include "String.h"

namespace hardware {

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

    cstd::string to_string() const;
};

class CpuInfo {
public:
	u64 rtdsc() const;
    cstd::string get_vendor() const;
    CpuMultimediaExtensions get_multimedia_extensions() const;

private:
    void __cpuid(int* cpuinfo, int info) const;
    unsigned long long _xgetbv(unsigned int index) const;
};

} /* namespace hardware */

#endif /* SRC_CPUINFO_H_ */
