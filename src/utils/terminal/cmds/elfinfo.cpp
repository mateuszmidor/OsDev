/**
 *   @file: elfinfo.cpp
 *
 *   @date: Sep 7, 2017
 * @author: Mateusz Midor
 */

#include "elfinfo.h"

#include "../../Elf64.h"
#include "kstd.h"

using namespace kstd;
using namespace utils;
using namespace filesystem;
namespace cmds {

void elfinfo::run() {
    if (env->volumes.empty()) {
        env->printer->format("elfinfo: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("elfinfo: please specify file name\n");
        return;
    }

    string filename = env->cwd + "/" + env->cmd_args[1];
    VolumeFat32* v = env->volume;
    auto e = v->get_entry(filename);
    if (!e) {
        env->printer->format("elfinfo: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory()) {
        env->printer->format("elfinfo: % is a directory\n", filename);
        return;
    }


    u32 size = e.get_size();
    char* buff = new char[size];
    e.read(buff, size);
    Elf64 elf;
    env->printer->format("% \n", elf.to_string(buff));

    delete[] buff;
}
} /* namespace cmds */
