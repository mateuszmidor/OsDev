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
    if (env->volumes.empty()) {
        env->printer->format("echo: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("echo: please specify file name\n");
        return;
    }
    string filename = env->cwd + "/" + env->cmd_args[1];
    VolumeFat32* v = env->volume;
    auto e = v->create_entry(filename, false);
    if (!e) {
        env->printer->format("echo: couldn't create file '%'\n", filename);
        return;
    }
    string number_str;
    for (u32 i = 0; i < 4096; i++) {
        number_str += "#";
    }
    e.write(number_str.data(), number_str.length());
    number_str = "!!!!!";
    e.write(number_str.data(), number_str.length());

//   number_str.reserve(256*4+1);    // 1024 chars
//    for (u32 i = 0; i < 256; i++) {
//        number_str += kstd::to_str(100 + i) + " ";
//    }
//
//    for (u32 i = 0; i < 128 * 4; i++)
//        v->write_file_entry(e, number_str.data(), number_str.length());

}
} /* namespace cmds */
