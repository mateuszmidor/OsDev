/**
 *   @file: upper.cpp
 *
 *   @date: Nov 23, 2017
 * @author: Mateusz Midor
 */

#include "_start.h"
#include "Cin.h"
#include "Cout.h"
#include "StringUtils.h"

using namespace cstd;
using namespace cstd::ustd;


/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    string line;

    cout::print(": ");
    line = cin::readln();

    while (!line.empty()) {
        string upper_line = StringUtils::to_upper_case(line);
        cout::format("> %\n", upper_line);
        cout::print(": ");
        line = cin::readln();
    };
    cout::print("upper done.\n");

    return 0;
}
