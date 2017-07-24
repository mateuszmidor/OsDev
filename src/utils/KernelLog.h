/**
 *   @file: KernelLog.h
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_KERNELLOG_H_
#define SRC_KERNELLOG_H_

#include "kstd.h"

class KernelLog {
public:
    static KernelLog& instance();
    const kstd::string& get_text() const;

    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename ... Args>
    void format(char const *fmt, Args ... args) {
        log_str += kstd::format(fmt, args...);
    }

private:
    KernelLog();
    static KernelLog _instance;
    kstd::string log_str;
};

#endif /* SRC_KERNELLOG_H_ */
