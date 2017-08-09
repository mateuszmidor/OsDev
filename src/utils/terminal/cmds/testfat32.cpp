/**
 *   @file: testfat32.cpp
 *
 *   @date: Aug 9, 2017
 * @author: Mateusz Midor
 */

#include "testfat32.h"

using namespace kstd;
using namespace filesystem;
namespace cmds {

void test_fat32::run() {
    if (env->volumes.empty())
        env->printer->format("No volumes installed\n");

    auto v = *env->volume;


    if (env->cmd_args.size() != 2) {
        env->printer->format("test_fat32: please specify action: -clean, -gen, -rem\n");
    }

    if (env->cmd_args[1] == "-clean")
        cleanup(v);

    if (env->cmd_args[1] == "-gen")
        generate(v);

    if (env->cmd_args[1] == "-rem")
        remove(v);
}

void test_fat32::cleanup(VolumeFat32& v) {
    for (u16 i = 1; i < 16; i++) {
          string name = string("/FILE_") +kstd::to_str(i);
          v.delete_entry(name);
    }

    v.delete_entry("/LEVEL1/LEVEL2/LEVEL3/LEVEL3.TXT");
    v.delete_entry("/LEVEL1/LEVEL2/LEVEL3");
    v.delete_entry("/LEVEL1/LEVEL2/LEVEL2.TXT");
    v.delete_entry("/LEVEL1/LEVEL2");
    v.delete_entry("/LEVEL1/LEVEL1.TXT");
    v.delete_entry("/LEVEL1");
    v.delete_entry("/TMP/TMP1.TXT");
    v.delete_entry("/TMP/TMP2.TXT");
    v.delete_entry("/TMP");
    v.delete_entry("/TO_BE_~1");
}

void test_fat32::generate(VolumeFat32& v) {
            for (u16 i = 1; i <= NUM_ENTRIES; i++) {
                  string name = string("/F_") +kstd::to_str(i);
                  v.create_entry(name, false);
            }
}

void test_fat32::remove(VolumeFat32& v) {
            for (u16 i = 1; i <= NUM_ENTRIES; i++) {
                  string name = string("/F_") +kstd::to_str(i);
                  v.delete_entry(name);
            }
}
} /* namespace cmds */
