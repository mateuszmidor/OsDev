/**
 *   @file: GlobalConstructorsRunner.cpp
 *
 *   @date: Jun 20, 2017
 * @author: Mateusz Midor
 */

#include "GlobalConstructorsRunner.h"

typedef void (*Constructor)();
extern "C" Constructor start_ctors;
extern "C" Constructor end_ctors;

namespace utils {


/**
 * @name    run
 * @brief   Call the constructors of objects defined in global namespace (if any)
 * @note    start_ctors and end_ctors defined in linker.ld
 */
void GlobalConstructorsRunner::run() {
    for (Constructor* c = &start_ctors; c != &end_ctors; c++)
        (*c)();
}

} // namespace utils
