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
 * @return  0 on failure
 *          task_id on success
 */
u32 elfrun::run(const CmdArgs& args) {
    if (args.size() < 2) {
        cout::print("elfrun: please specify file name\n");
        return 0;
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
    }

    delete[] nullterm_argv;
    return (elf_run_result > 0) ? elf_run_result : 0;
}

} /* namespace cmds */
