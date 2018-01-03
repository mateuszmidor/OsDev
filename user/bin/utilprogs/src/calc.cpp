/**
 *   @file: calc.cpp
 *
 *   @date: Dec 21, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "Cout.h"
#include "ReversePolishNotation.h"

using namespace ustd;


/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print("calc: please speficy formula like 2 + 4 * 5\n");
        return 1;
    }

    string formula {};
    for (int i = 1; i < argc; i++)
        formula += argv[i];

    if (auto result = rpn::Calculator::calc(formula))
        cout::format("RESULT: %\n", result.value);
    else
        cout::format("ERROR: %\n", result.error_msg);

    return 0;
}

