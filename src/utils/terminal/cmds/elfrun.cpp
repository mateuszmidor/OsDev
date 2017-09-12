/**
 *   @file: elfrun.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "elfrun.h"
#include "kstd.h"
#include "Elf64.h"
#include "TaskManager.h"

using namespace kstd;
using namespace utils;
using namespace filesystem;
using namespace multitasking;
namespace cmds {

void elfrun::run() {
    if (env->volumes.empty()) {
        env->printer->format("elfrun: no volumes installed\n");
        return;
    }

    if (env->cmd_args.size() < 2) {
        env->printer->format("elfrun: please specify file name\n");
        return;
    }

    string name = env->cmd_args[1];
    u64 arg = 0;
    if (env->cmd_args.size() > 2) {
        arg = str_to_long(env->cmd_args[2].c_str());
    }

    string filename = env->cwd + "/" + name;
    VolumeFat32* v = env->volume;
    auto e = v->get_entry(filename);
    if (!e) {
        env->printer->format("elfrun: file % doesnt exist\n", filename);
        return;
    }

    if (e.is_directory()) {
        env->printer->format("elfrun: % is a directory\n", filename);
        return;
    }


    u32 size = e.get_size();
    char* buff = new char[size];
    e.read(buff, size);

    TaskEntryPoint entry_point = (TaskEntryPoint)Elf64::load_into_current_addressspace(buff);
    TaskManager& task_manager = TaskManager::instance();
    auto task = std::make_shared<Task>(entry_point, name, arg, true);
    task_manager.add_task(task);

    delete[] buff;
}

} /* namespace cmds */
