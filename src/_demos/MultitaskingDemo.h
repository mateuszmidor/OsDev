/*
 * MultitaskingDemo.h
 *
 *  Created on: Jul 23, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_MULTITASKINGDEMO_H_
#define SRC__DEMOS_MULTITASKINGDEMO_H_

#include "ScrollableScreenPrinter.h"

namespace demos {

class MultitaskingDemo {
public:
    MultitaskingDemo();
    void run(u64 arg);

private:
    static utils::LimitedAreaScreenPrinter printer;
};


} /* namespace demos */

#endif /* SRC__DEMOS_MULTITASKINGDEMO_H_ */
