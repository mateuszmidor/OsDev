/**
 *   @file: ls.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "ls.h"
#include "syscalls.h"

using namespace ustd;
using namespace middlespace;
namespace cmds {

void ls::run() {
    string path;
    if (env->cmd_args.size() == 1)
        path = env->cwd;
    else
        path = make_absolute_filename(env->cmd_args[1]);


    // check exists
    struct stat s;
    if (syscalls::stat(path.c_str(), &s) == -1) {
        env->printer->format("ls: path '%' does not exist\n", path);
        return;
    }

    // ls file
    if (s.st_mode == S_IFREG) {
        env->printer->format("% - %B\n", env->cmd_args[1], s.st_size);
        return;
    }


    // ls directory
    int fd = syscalls::open(path.c_str());
    u32 MAX_ENTRIES = 128; // should there be more in a single dir?
    FsEntry* entries = new FsEntry[MAX_ENTRIES];

    int count = syscalls::enumerate(fd, entries, MAX_ENTRIES);
    for (int i = 0; i < count; i++)
        if (entries[i].is_directory)
            env->printer->format("[%]\n", entries[i].name);
        else
            env->printer->format("% - %B\n", entries[i].name, entries[i].size);

    delete[] entries;
    syscalls::close(fd);
}
} /* namespace cmds */
