/**
 *   @file: echo.cpp
 *
 *   @date: Jul 31, 2017
 * @author: Mateusz Midor
 */

#include "echo.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void echo::run() {
    if (env->cmd_args.size() < 2) {
        env->printer->format("echo: please specify file name\n");
        return;
    }

    string filename = make_absolute_filename(env->cmd_args[1]);
    VfsEntryPtr e = env->vfs_manager.create_entry(filename, false);
    if (!e) {
        env->printer->format("echo: couldn't create file '%'\n", filename);
        return;
    }
    string number_str;

    u32 count;
    if (env->cmd_args.size() == 3)
        count = str_to_long(env->cmd_args[2].c_str());
    else
        count = 64;

    if (count == 0)
        return;

    for (u32 i = 0; i < count; i++) {
        number_str += "#";
    }

    env->klog->format("echo: writing % bytes of data\n", number_str.length());
    e->write(number_str.data(), number_str.length());

    number_str = "!!!";
    env->klog->format("echo: writing '!!!' at the end\n", number_str.length());
    e->write(number_str.data(), number_str.length());
}
} /* namespace cmds */
