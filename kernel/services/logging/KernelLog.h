/**
 *   @file: KernelLog.h
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_KERNELLOG_H_
#define SRC_KERNELLOG_H_

#include "StringUtils.h"

namespace logging {

class KernelLog {
public:
    static KernelLog& instance();
    const cstd::string& get_text() const;
    void clear();

    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename ... Args>
    void format(const cstd::string& fmt, Args ... args) {
        log_str += cstd::StringUtils::format(fmt, args...);
    }

    void put(const cstd::string& s) {
    	log_str += s;
    }

private:
    KernelLog();
    static KernelLog _instance;
    cstd::string log_str;
};

} // namespace logging
#endif /* SRC_KERNELLOG_H_ */
