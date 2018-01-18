/**
 *   @file: KernelLog.cpp
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#include "KernelLog.h"

namespace logging {

KernelLog KernelLog::_instance;

KernelLog::KernelLog() {
}

KernelLog& KernelLog::instance() {
    return _instance;
}

const cstd::string& KernelLog::get_text() const {
    return log_str;
}

void KernelLog::clear() {
    log_str.clear();
}
} // namespace logging
