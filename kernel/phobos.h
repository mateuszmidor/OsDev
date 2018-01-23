/**
 *   @file: phobos.h
 *
 *   @date: Jan 23, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_PHOBOS_H_
#define KERNEL_PHOBOS_H_

#include "DriverManager.h"
#include "TimeManager.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "VfsManager.h"
#include "VgaPrinter.h"
#include "ElfRunner.h"

/**
 * @brief   Phobos public interface
 */
namespace phobos {
    using InitTaskPtr = void(*)();

    extern utils::VgaPrinter            printer;
    extern drivers::DriverManager&      driver_manager;
    extern filesystem::VfsManager&      vfs_manager;
    extern ktime::TimeManager&          time_manager;
    extern multitasking::TaskManager&   task_manager;

    void boot_and_start_multitasking(void *multiboot2_info_ptr, const InitTaskPtr init_task);
    void halt();
}


#endif /* KERNEL_PHOBOS_H_ */
