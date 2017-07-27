/**
 *   @file: lscpu.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "lscpu.h"
#include "CpuInfo.h"

namespace cmds {

void lscpu::run() {
    utils::CpuInfo cpu_info;
    env->printer->format("CPU: % @ %MHz\n", cpu_info.get_vendor(), cpu_info.get_peak_mhz());
}

} /* namespace cmds */
