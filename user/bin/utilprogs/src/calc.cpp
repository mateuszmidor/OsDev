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

string build_formula(int argc, char* argv[]) {
    string formula;
    for (int i = 1; i < argc; i++)
        formula += argv[i];
    return std::move(formula);
}

/**
 * @brief   Entry point
 * @return  0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout::print("calc: please speficy formula like 2 + 4 * 5. use sin/cos/sqrt/abs\n");
        return 1;
    }

    string formula = build_formula(argc, argv);

    rpn::Calculator c;
    auto parse_result = c.parse(formula);
    if (!parse_result) {
        cout::format("calc: parse error: %\n", parse_result.error_msg);
        return 1;
    }

    c.define("e", 2.718281828459);
    auto calc_result = c.calc();
    if (!calc_result) {
        cout::format("calc: calculation error: %\n", calc_result.error_msg);
        return 1;
    }

    cout::format("RESULT: %\n", calc_result.value);
    return 0;
}

