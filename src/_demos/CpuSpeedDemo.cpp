/**
 *   @file: CpuSpeedDemo.cpp
 *
 *   @date: Jul 24, 2017
 * @author: Mateusz Midor
 */

#include "CpuSpeedDemo.h"
#include "CpuInfo.h"

using namespace utils;

namespace demos {

void CpuSpeedDemo::run(u64 arg) {
    CpuInfo info;
    info.print_to_klog();
}


} /* namespace demos */
