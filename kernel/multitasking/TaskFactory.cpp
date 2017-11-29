/**
 *   @file: TaskFactory.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "TaskFactory.h"
#include "HigherHalf.h"
#include "PageTables.h"

using namespace memory;
namespace multitasking {

TaskGroupData    TaskFactory::kernel_task_group(PageTables::get_kernel_pml4_phys_addr(), "/",
                                                HigherHalf::get_kernel_heap_low_limit(), HigherHalf::get_kernel_heap_high_limit());

TaskGroupDataPtr TaskFactory::kernel_task_group_ptr;

} /* namespace multitasking */
