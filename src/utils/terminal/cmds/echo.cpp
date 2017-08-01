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
    SimpleDentryFat32 e;
//    if (!v->get_entry(filename, e)) {
//        env->printer->format("echo: file % doesnt exist\n", filename);
//        return;
//    }
//
//    if (e.is_directory) {
//        env->printer->format("cat: % is a directory\n", filename);
//        return;
//    }

    v->create_entry(filename, false, e);
    string number_str;

    number_str.reserve(52450);
    for (u32 i = 0; i < 1024 * 8; i++){
        number_str += kstd::to_str(1000 + i) + " ";
    }

    for (u32 i = 0; i < 128 * 1; i++)
        v->write_file_entry(e, number_str.data(), number_str.length());


//    for (u32 i = 0; i < 1024 * 10; i++){
//        number_str = kstd::to_str(1000 + i) + " ";
//        if (0 == v->write_file_entry(e, number_str.data(), number_str.length()))
//            break;
//    }

}
} /* namespace cmds */
