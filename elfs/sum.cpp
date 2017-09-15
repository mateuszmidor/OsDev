/**
 *   @file: square.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "utils.h"


/**
 * @brief   Entry point
 * @return  Sum of provided integer arguments
 */
int main(int argc, char* argv[]) {
    if (argc < 2)
        return -1;

    int result = 0;
    for (int i = 1; i < argc; i++)
        result += str_to_long(argv[i]);

    return result;
}
