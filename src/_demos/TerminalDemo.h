/*
 * TerminalDemo.h
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_TERMINALDEMO_H_
#define SRC__DEMOS_TERMINALDEMO_H_

#include "ScreenPrinter.h"
#include "KeyboardDriver.h"

namespace demos {

class TerminalDemo {
public:
    void run();

private:
    void on_key_press(drivers::Key key);
    ScrollableScreenPrinter printer;
};

} /* namespace demos */

#endif /* SRC__DEMOS_TERMINALDEMO_H_ */
