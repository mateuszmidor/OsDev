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
    cwd = "/mnt/PHOBOS_D";

//    if (vfs_manager.get_entry("/mnt/PHOBOS_D"))
//        cwd = "/mnt/PHOBOS_D";
//    else if (vfs_manager.get_entry("/mnt"))
//        cwd = "/mnt";
//    else if (vfs_manager.get_entry("/"))
//        cwd = "/";
//    else
//        cwd = "[NO FILESYSTEM ROOT IS INSTALLED]";

}
} /* namespace terminal */
