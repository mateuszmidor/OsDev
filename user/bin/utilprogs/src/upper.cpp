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

using namespace ustd;


/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {

    cout::print("\n");
    string line;
    do {
        cout::print("> ");
        line = cin::readln();
        string upper_line = StringUtils::to_upper_case(line);
        cout::print(upper_line);
    } while (line != "");

    cout::print("\nupper done.");
    return 0;
}
