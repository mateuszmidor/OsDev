/**
 *   @file: ls.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "ls.h"
#include "VfsManager.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

//void ls::run() {
//    if (env->volumes.empty())
//        env->printer->format("No volumes installed\n");
//
//
//    OnEntryFound on_entry = [&](const Fat32Entry& e) -> bool {
//        if (e.is_directory())
//            env->printer->format("[%]\n", e.get_name());
//        else
//            env->printer->format("% - %B\n", e.get_name(), e.get_size());
//
//        return true;
//    };
//
//    string path;
//    if (env->cmd_args.size() == 2)
//        path = env->cwd + "/" + env->cmd_args[1];
//    else
//        path = env->cwd;
//
//    VolumeFat32* v = env->volume;
//    auto e = v->get_entry(path);
//    if (!e) {
//        env->printer->format("ls: path '%' does not exist\n", path);
//        return;
//    }
//
//    if (e.is_directory())
//        e.enumerate_entries(on_entry);
//    else
//        env->printer->format("% - %B\n", e.get_name(), e.get_size());
//
//}

void ls::run() {
    OnVfsEntryFound on_entry = [&](VfsEntryPtr e) -> bool {
        if (e->is_directory())
            env->printer->format("[%]\n", e->get_name());
        else
            env->printer->format("% - %B\n", e->get_name(), e->get_size());

        return true;
    };

    string path;
    if (env->cmd_args.size() == 2)
        if (env->cmd_args[1][0] == '/')
            path = env->cmd_args[1];
        else
            path = env->cwd + "/" + env->cmd_args[1];
    else
        path = env->cwd;

    VfsManager& vfs = VfsManager::instance();
    VfsEntryPtr e = vfs.get_entry(path);

    if (!e) {
        env->printer->format("ls: path '%' does not exist\n", path);
        return;
    }

    if (e->is_directory())
        e->enumerate_entries(on_entry);
    else
        env->printer->format("% - %B\n", e->get_name(), e->get_size());

}
} /* namespace cmds */
