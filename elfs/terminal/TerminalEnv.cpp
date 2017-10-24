/**
 *   @file: TerminalEnv.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "TerminalEnv.h"

using namespace ustd;
namespace terminal {

TerminalEnv::TerminalEnv() :
    printer(nullptr) {
    cwd = "/HOME";
}
} /* namespace terminal */
