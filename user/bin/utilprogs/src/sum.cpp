/**
 *   @file: square.cpp
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "utils.h"
#include "StringUtils.h"

/**
 * @brief   Entry point
 * @return  Sum of provided integer arguments on success, -1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2)
        return -1;

    int result = 0;
    for (int i = 1; i < argc; i++)
        result += ustd::StringUtils::to_int(argv[i]);

    return result;
}
