/**
 *   @file: CommandHistory.h
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_TERMINAL_COMMANDHISTORY_H_
#define SRC_UTILS_TERMINAL_COMMANDHISTORY_H_

#include "kstd.h"

namespace terminal {

class CommandHistory {
public:
    CommandHistory();
    void append(const kstd::string& cmd);
    void set_to_latest();
    const kstd::string& get_prev();
    const kstd::string& get_next();

private:
    kstd::vector<kstd::string> history;
    u16 index   {0};
};
} /* namespace terminal */

#endif /* SRC_UTILS_TERMINAL_COMMANDHISTORY_H_ */
