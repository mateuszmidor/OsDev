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
    kstd::string multimedia_extension_string = cpu_info.get_multimedia_extensions().to_string();
    env->printer->format("CPU: % @ %MHz, %\n", cpu_info.get_vendor(), cpu_info.get_peak_mhz(), multimedia_extension_string);
}

} /* namespace cmds */
