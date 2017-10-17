/**
 *   @file: TerminalEnv.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "TerminalEnv.h"

#include "MassStorageMsDos.h"
#include "DriverManager.h"

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace filesystem;
namespace terminal {

TerminalEnv::TerminalEnv() :
    printer(nullptr), vfs_manager(VfsManager::instance()) {

    if (vfs_manager.get_entry("/mnt"))
        cwd = "/mnt";
    else if (vfs_manager.get_entry("/"))
        cwd = "/";
    else
        cwd = "[NO FILESYSTEM ROOT IS INSTALLED]";

}
} /* namespace terminal */
