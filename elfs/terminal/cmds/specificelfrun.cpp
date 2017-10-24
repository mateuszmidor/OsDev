/**
 *   @file: specificelfrun.cpp
 *
 *   @date: Oct 24, 2017
 * @author: Mateusz Midor
 */

#include "specificelfrun.h"

namespace cmds {

specificelfrun::specificelfrun(terminal::TerminalEnv* arg, const ustd::string& elf_absolute_path) :
        CmdBase::CmdBase(env), elf_path(elf_absolute_path){
}

void specificelfrun::run() {
    env->printer->format("specificelfrun: running %\n", elf_path);
    u32 count = env->cmd_args.size();
    const char** nullterm_argv = new const char*[count + 1]; // +1 for list terminating null
    for (u32 i = 0; i < count; i++) {
        env->printer->format("specificelfrun: arg: %\n", env->cmd_args[i]);
        nullterm_argv[i] = env->cmd_args[i].c_str();
    }

    nullterm_argv[count] = nullptr;

    // run the elf
    switch (syscalls::elf_run(elf_path.c_str(), nullterm_argv)) {
    case -ENOENT:
        env->printer->format("specificelfrun: no such file\n");
        break;

    case -EISDIR:
        env->printer->format("specificelfrun: given filename points to a directory not a file\n");
        break;

    case -ENOMEM:
        env->printer->format("specificelfrun: no enough memory to run ELF\n");
        break;

    case -ENOEXEC:
        env->printer->format("specificelfrun: give filename points to non-executable\n");
        break;

    case -EPERM:
        env->printer->format("specificelfrun: task mamanger didnt allow to run new task. Too many tasks is running probably\n");
        break;

    default:
        break;
    }

    delete[] nullterm_argv;
}
} /* namespace cmds */
