/**
 *   @file: elfrun.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "elfrun.h"
#include "syscalls.h"
#include "Cout.h"

using namespace cstd;
using namespace cstd::ustd;

namespace cmds {

/**
 * ELF image is organized as follows:
 * 0...x -> process image
 * ELF_VIRTUAL_MEM_BYTES - stack size -> process stack
 */
void elfrun::run(const CmdArgs& args, bool run_in_bg) {
    if (args.size() < 2) {
        cout::print("elfrun: please specify file name\n");
        return;
    }

    string filename = args[1];

    u32 count = args.size() -1; // no first one
    const char** nullterm_argv = new const char*[count + 1]; // +1 for list terminating null
    for (u32 i = 0; i < count; i++)
        nullterm_argv[i] = args[i+1].c_str();

    nullterm_argv[count] = nullptr;

    // run the elf
    s64 elf_run_result = syscalls::elf_run(filename.c_str(), nullterm_argv);
    switch (elf_run_result) {
    case -ENOENT:
        cout::print("elfrun: no such file\n");
        break;

    case -EISDIR:
        cout::print("elfrun: given filename points to a directory not a file\n");
        break;

    case -ENOMEM:
        cout::print("elfrun: no enough memory to run ELF\n");
        break;

    case -ENOEXEC:
        cout::print("elfrun: give filename points to non-executable\n");
        break;

    case -EPERM:
        cout::print("elfrun: task mamanger didnt allow to run new task. Too many tasks is running probably\n");
        break;

    default:
        if (!run_in_bg)
            syscalls::task_wait(elf_run_result); // wait till task exits. This deadlocks if task writes to "/dev/stdout, fills it up and blocks waiting for someone to read
        break;
    }

    delete[] nullterm_argv;
}

} /* namespace cmds */
