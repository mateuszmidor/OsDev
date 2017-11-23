/**
 *   @file: main.cpp
 *
 *   @date: Sep 20, 2017
 * @author: Mateusz Midor
 */


#include "_start.h"
#include "utils.h"
#include "Terminal.h"


using namespace terminal;
int main(int argc, char* argv[]) {
    Terminal t(0);
    t.run();
    print("Goodbye terminal");
    return 0;
}
