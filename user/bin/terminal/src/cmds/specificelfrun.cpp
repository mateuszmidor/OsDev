/**
 *   @file: specificelfrun.cpp
 *
 *   @date: Oct 24, 2017
 * @author: Mateusz Midor
 */

#include "Cout.h"
#include "syscalls.h"
#include "specificelfrun.h"

using namespace cstd;
using namespace cstd::ustd;
namespace cmds {

specificelfrun::specificelfrun(const string& elf_absolute_path) :
        elf_path(elf_absolute_path){
}

u32 specificelfrun::run(const CmdArgs& args) {
    u32 count = args.size();
    const char** nullterm_argv = new const char*[count + 1]; // +1 for list terminating null
    for (u32 i = 0; i < count; i++)
        nullterm_argv[i] = args[i].c_str();

    nullterm_argv[count] = nullptr;

    // run the elf
    s64 elf_run_result = syscalls::elf_run(elf_path.c_str(), nullterm_argv);
    switch (elf_run_result) {
    case -ENOENT:
        cout::print("specificelfrun: no such file\n");
        break;

    case -EISDIR:
        cout::print("specificelfrun: given filename points to a directory not a file\n");
        break;

    case -ENOMEM:
        cout::print("specificelfrun: no enough memory to run ELF\n");
        break;

    case -ENOEXEC:
        cout::format("specificelfrun: given filename points to non-executable: %\n", elf_path);
        break;

    case -EPERM:
        cout::print("specificelfrun: task mananger didnt allow to run new task. Too many tasks is running probably\n");
        break;
    }

    delete[] nullterm_argv;
    return (elf_run_result > 0) ? elf_run_result : 0;
}
} /* namespace cmds */
