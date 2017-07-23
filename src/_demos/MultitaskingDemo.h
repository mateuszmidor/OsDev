/*
 * MultitaskingDemo.h
 *
 *  Created on: Jul 23, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_MULTITASKINGDEMO_H_
#define SRC__DEMOS_MULTITASKINGDEMO_H_

#include "ScreenPrinter.h"

namespace demos {

class MultitaskingDemo {
public:
    MultitaskingDemo(char c_to_print);
    void run();

private:
    char c_to_print;
    static BoundedAreaScreenPrinter printer;
};

class MultitaskingDemoA : public MultitaskingDemo {
public:
    MultitaskingDemoA() : MultitaskingDemo('A') {}
};

class MultitaskingDemoB : public MultitaskingDemo {
public:
    MultitaskingDemoB() : MultitaskingDemo('B') {}
};

} /* namespace demos */

#endif /* SRC__DEMOS_MULTITASKINGDEMO_H_ */
