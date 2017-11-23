/**
 *   @file: CommandHistory.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_COMMANDHISTORY_H_
#define SRC_UTILS_TERMINAL_COMMANDHISTORY_H_

#include "types.h"
#include "String.h"
#include "Vector.h"

namespace terminal {

class CommandHistory {
public:
    CommandHistory();
    void append(const ustd::string& cmd);
    void set_to_latest();
    const ustd::string& get_prev();
    const ustd::string& get_next();

private:
    ustd::vector<ustd::string> history;
    u16 index   {0};
};
} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_COMMANDHISTORY_H_ */
