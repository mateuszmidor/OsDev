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
    static void make_demo_() {
        T demo;
        demo.run();
    }

    template <class T>
    static std::shared_ptr<Task> make_demo(const kstd::string& name) {
        return std::make_shared<Task>(make_demo_<T>, name);
    }
};

} /* namespace demos */

#endif /* SRC__DEMOS_DEMO_H_ */
