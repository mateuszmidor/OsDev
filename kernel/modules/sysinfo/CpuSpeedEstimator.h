/*
 * CpuSpeedEstimator.h
 *
 *  Created on: Dec 21, 2018
 *      Author: Mateusz Midor
 */

#ifndef KERNEL_MODULES_SYSINFO_CPUSPEEDESTIMATOR_H_
#define KERNEL_MODULES_SYSINFO_CPUSPEEDESTIMATOR_H_

#include "kstd.h"
#include "Optional.h"

namespace sysinfo {
namespace cpuspeedestimator {

cstd::Optional<u32> estimate_peak_mhz();

} /* namespace cpuspeedestimator */
} /* namespace sysinfo */

#endif /* KERNEL_MODULES_SYSINFO_CPUSPEEDESTIMATOR_H_ */
