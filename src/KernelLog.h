/**
 *   @file: KernelLog.h
 *
 *   @date: Jul 18, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_KERNELLOG_H_
#define SRC_KERNELLOG_H_

#include "ScreenPrinter.h"

class KernelLog {
public:
    static KernelLog& instance();
    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename ... Args>
    void format(char const *fmt, Args ... args) {
        ScreenPrinter& printer = ScreenPrinter::instance();
        printer.format(fmt, args...);
    }

private:
    static KernelLog _instance;
};

#endif /* SRC_KERNELLOG_H_ */
