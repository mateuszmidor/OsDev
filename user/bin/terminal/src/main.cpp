/**
 *   @file: main.cpp
 *
 *   @date: Sep 20, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "Terminal.h"

using namespace terminal;

int main(int argc, char* argv[]) {
    Terminal t(argv[0]);
    t.run();
    return 0;
}
