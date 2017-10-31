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
 * @return  argv[1] * argv[1]
 */
int main(int argc, char* argv[]) {
    if (argc < 2)
        return 1;

    int value = str_to_long(argv[1]);
    return value * value;
}
