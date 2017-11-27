/**
 *   @file: Demo.h
 *
 *   @date: Jul 21, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_DEMO_H_
#define SRC__DEMOS_DEMO_H_

#include "kstd.h"
#include "Task.h"

namespace demos {

class Demo {
public:
    template <class T>
    static void make_demo_(u64 arg) {
        T demo;
        demo.run(arg);
    }

    template <class T>
    static multitasking::Task* make_demo(const kstd::string& name, u64 arg = 0) {
        multitasking::Task* task = multitasking::Task::make_kernel_task(make_demo_<T>, name.c_str())->set_arg1(arg);
        return task;
    }
};

} /* namespace demos */

#endif /* SRC__DEMOS_DEMO_H_ */
