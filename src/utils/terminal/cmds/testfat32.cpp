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
    if (env->cmd_args.size() != 2) {
        env->printer->format("test_fat32: please specify action: -clean, -gen, -rem\n");
    }

    if (env->cmd_args[1] == "-clean")
        cleanup();

    if (env->cmd_args[1] == "-gen")
        generate();

    if (env->cmd_args[1] == "-rem")
        remove();
}

void test_fat32::cleanup() {
    VfsManager& vfs = env->vfs_manager;

    for (u16 i = 1; i < 16; i++) {
          string name = env->cwd + string("/FILE_") +kstd::to_str(i);
          vfs.delete_entry(name);
    }

    vfs.delete_entry(env->cwd + "/LEVEL1/LEVEL2/LEVEL3/LEVEL3.TXT");
    vfs.delete_entry(env->cwd + "/LEVEL1/LEVEL2/LEVEL3");
    vfs.delete_entry(env->cwd + "/LEVEL1/LEVEL2/LEVEL2.TXT");
    vfs.delete_entry(env->cwd + "/LEVEL1/LEVEL2");
    vfs.delete_entry(env->cwd + "/LEVEL1/LEVEL1.TXT");
    vfs.delete_entry(env->cwd + "/LEVEL1");
    vfs.delete_entry(env->cwd + "/TMP/TMP1.TXT");
    vfs.delete_entry(env->cwd + "/TMP/TMP2.TXT");
    vfs.delete_entry(env->cwd + "/TMP");
    vfs.delete_entry(env->cwd + "/TO_BE_~1");
}

void test_fat32::generate() {
    VfsManager& vfs = env->vfs_manager;
    for (u16 i = 1; i <= NUM_ENTRIES; i++) {
          string name = string(env->cwd + "/F_") +kstd::to_str(i);
          vfs.create_entry(name, false);
    }
}

void test_fat32::remove() {
    VfsManager& vfs = env->vfs_manager;
    for (u16 i = 1; i <= NUM_ENTRIES; i++) {
          string name = string(env->cwd + "/F_") +kstd::to_str(i);
          vfs.delete_entry(name);
    }
}
} /* namespace cmds */
