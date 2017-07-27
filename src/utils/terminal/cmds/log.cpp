/**
 *   @file: log.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "log.h"

namespace cmds {

void log::run() {
    env->printer->format("%\n", env->klog->get_text());
}
} /* namespace cmds */
