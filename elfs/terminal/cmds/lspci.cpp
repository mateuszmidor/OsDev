/**
 *   @file: lspci.cpp
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#include "PCIController.h"
#include "lspci.h"

using namespace hardware;
namespace cmds {

void lspci::run() {
    PCIController pcic;
    env->printer->format(pcic.drivers_to_string());
}
} /* namespace cmds */
