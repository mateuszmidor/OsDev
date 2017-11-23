/**
 *   @file: Cout.h
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_COUT_H_
#define USER_USTD_SRC_COUT_H_

#include "types.h"
#include "StringUtils.h"

namespace ustd {

class cout {
public:
    static void print(const string& s);
    static void print(const char str[], size_t count);

    template<typename ... Args>
    static void format(const string& fmt, Args ... args) {
        print(StringUtils::format(fmt, args...));
    }

private:
    static int stdout_fd;
    static void _print(int fd, const char str[], size_t len);
};

} /* namespace ustd */

#endif /* USER_USTD_SRC_COUT_H_ */
