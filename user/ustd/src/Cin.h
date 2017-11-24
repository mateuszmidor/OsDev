/**
 *   @file: Cin.h
 *
 *   @date: Nov 24, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_CIN_H_
#define USER_USTD_SRC_CIN_H_

#include "String.h"

namespace ustd {

class cin {
public:
    static string readln();

private:
    static int stdin_fd;
};

} /* namespace ustd */

#endif /* USER_USTD_SRC_CIN_H_ */
