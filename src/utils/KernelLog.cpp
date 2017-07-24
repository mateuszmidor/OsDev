/**
 *   @file: KernelLog.cpp
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#include "KernelLog.h"

namespace utils {

KernelLog KernelLog::_instance;

KernelLog::KernelLog() {
}

KernelLog& KernelLog::instance() {
    return _instance;
}

const kstd::string& KernelLog::get_text() const {
    return log_str;
}

} // namespace utils
