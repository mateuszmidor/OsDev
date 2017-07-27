/**
 *   @file: TaskFactory.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MULTITASKING_TASKFACTORY_H_
#define SRC_MULTITASKING_TASKFACTORY_H_

#include "Task.h"
#include <memory>

namespace multitasking {

class TaskFactory {
public:
    /**
     * @brief   Makes a shared_ptr<Task> object from a class that exposes Constructor(u64 arg) and void run()
     */
    template <class T>
    static std::shared_ptr<multitasking::Task> make(const kstd::string& name, u64 arg = 0) {
        return std::make_shared<Task>(make_<T>, name, arg);
    }

private:
    template <class T>
    static void make_(u64 arg) {
        T task(arg);
        task.run();
    }
};

} /* namespace multitasking */

#endif /* SRC_MULTITASKING_TASKFACTORY_H_ */
