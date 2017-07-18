/**
 *   @file: KernelLog.cpp
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#include "KernelLog.h"

KernelLog KernelLog::_instance;

KernelLog::KernelLog() :
        printer(1, 1, 88, 28, 90, 30) {
}

KernelLog& KernelLog::instance() {
    return _instance;
}
