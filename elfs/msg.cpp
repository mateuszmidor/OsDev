/**
 *   @file: msg.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "string.h"
#include "_start.h"
#include "syscalls.h"


/**
 * @brief   Entry point
 * @return  0 no matter what
 */
int main(int argc, char* argv[]) {
    const char STR[] = "msg: Hello from user space 'msg' binary!!\n";
    syscalls::write(2, STR, strlen(STR));
    return 0;
}
