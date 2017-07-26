/**
 *   @file: Demo.h
 *
 *   @date: Jul 21, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_DEMO_H_
#define SRC__DEMOS_DEMO_H_

#include <memory>
#include "kstd.h"
#include "TaskManager.h"

namespace demos {

class Demo {
public:
    template <class T>
    static void make_demo_(u64 arg) {
        T demo;
        demo.run(arg);
    }

    template <class T>
    static std::shared_ptr<multitasking::Task> make_demo(const kstd::string& name, u64 arg = 0) {
        return std::make_shared<multitasking::Task>(make_demo_<T>, name, arg);
    }
};

} /* namespace demos */

#endif /* SRC__DEMOS_DEMO_H_ */
