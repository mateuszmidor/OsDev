/**
 *   @file: ret3.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "_start.h"


/**
 * @brief   Entry point
 * @return  Simply return number 3
 */
int main(int argc, char* argv[]) {
    const char STR[] = "Hello from user space ret3 binary!!";
    write(2, STR, strlen(STR));
    return 3;
}
