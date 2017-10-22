/**
 *   @file: elfrun.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "elfrun.h"
#include "syscalls.h"

using namespace ustd;

namespace cmds {

/**
 * ELF image is organized as follows:
 * 0...x -> process image
 * ELF_VIRTUAL_MEM_BYTES - stack size -> process stack
 */
void elfrun::run() {
    if (env->cmd_args.size() < 2) {
        env->printer->format("elfrun: please specify file name\n");
        return;
    }

    string filename = make_absolute_filename(env->cmd_args[1]);

    u32 count = env->cmd_args.size() -1; // no first one
    const char** nullterm_argv = new const char*[count + 1]; // +1 for list terminating null
    for (u32 i = 0; i < count; i++)
        nullterm_argv[i] = env->cmd_args[i+1].c_str();

    nullterm_argv[count] = nullptr;

    // run the elf
    switch (syscalls::elf_run(filename.c_str(), nullterm_argv)) {
    case -ENOENT:
        env->printer->format("elfrun: no such file\n");
        break;

    case -EISDIR:
        env->printer->format("elfrun: given filename points to a directory not a file\n");
        break;

    case -ENOMEM:
        env->printer->format("elfrun: no enough memory to run ELF\n");
        break;

    default:
        break;
    }

    delete[] nullterm_argv;
}

} /* namespace cmds */
