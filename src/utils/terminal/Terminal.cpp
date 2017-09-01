/**
 *   @file: Terminal.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include "Terminal.h"
#include "ScrollableScreenPrinter.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "TaskManager.h"
#include "CpuInfo.h"

#include "cmds/df.h"
#include "cmds/pwd.h"
#include "cmds/log.h"
#include "cmds/lscpu.h"
#include "cmds/ls.h"
#include "cmds/cat.h"
#include "cmds/ps.h"
#include "cmds/free.h"
#include "cmds/cd.h"
#include "cmds/rm.h"
#include "cmds/mv.h"
#include "cmds/echo.h"
#include "cmds/mkdir.h"
#include "cmds/tail.h"
#include "cmds/trunc.h"
#include "cmds/testfat32.h"
#include "cmds/lspci.h"
#include "cmds/date.h"
#include <algorithm>
#include <memory>

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace multitasking;

namespace terminal {

const string Terminal::PROMPT {"> "};

Terminal::Terminal(u64 arg) :
        printer(0, 0, 89, 29),
        klog(KernelLog::instance()) {

    env.printer = &printer;
    env.klog = &klog;

    // user space
    install_cmd<cmds::pwd>("pwd", true);
    install_cmd<cmds::log>("klog", true);
    install_cmd<cmds::lscpu>("lscpu", true);
    install_cmd<cmds::ps>("ps", true);
    install_cmd<cmds::free>("free", true);
    install_cmd<cmds::lspci>("lspci", true);
    install_cmd<cmds::date>("date", true);

    // kernel space, to be moved to user space once system call interface is complete
    install_cmd<cmds::df>("df", false);
    install_cmd<cmds::ls>("ls", false);
    install_cmd<cmds::cat>("cat", false);
    install_cmd<cmds::cd>("cd", false);
    install_cmd<cmds::rm>("rm", false);
    install_cmd<cmds::mv>("mv", false);
    install_cmd<cmds::echo>("echo", false);
    install_cmd<cmds::mkdir>("mkdir", false);
    install_cmd<cmds::tail>("tail", false);
    install_cmd<cmds::trunc>("trunc", false);
    install_cmd<cmds::test_fat32>("test_fat32", false);
}

void Terminal::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        printer.format("/%% %", env.volume->get_label(), env.cwd, PROMPT);

        // get command
        string cmd = get_line();

        // process command
        process_cmd(cmd);
    };
}

bool Terminal::init() {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();

    auto keyboard = driver_manager.get_driver<KeyboardDriver>();
    if (!keyboard) {
        klog.format("VgaDemo::run: no KeyboardDriver\n");
        return false;
    }

    keyboard->set_on_key_press([&](Key key) { on_key_press(key); });
    printer.clear_screen();

    return true;
}

string Terminal::get_line() {
    Key key;
    do  {
        key = get_key();
        process_key(key);
    } while (key != Key::Enter);

    return edit_line;
}

Key Terminal::get_key() {
    // wait until last_key is set by keyboard interrupt. Keys might be missed if stroking faster than Terminal gets CPU time :)
    while (last_key == Key::INVALID)
        Task::yield();

    Key key = last_key;
    last_key = Key::INVALID;
    return key;
}

void Terminal::suggest_cmd(const string& cmd) {
    for (u16 i = 0; i < edit_line.length(); i++)
        printer.backspace();

    printer.format(cmd);
    edit_line = cmd;
}

void Terminal::process_key(Key key) {
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up: {
            suggest_cmd(cmd_history.get_prev());
            break;
        }

        case Key::Down: {
            suggest_cmd(cmd_history.get_next());
            break;
        }

        case Key::Tab: {
            bool multiple_results;
            string filter_result;
            std::tie(multiple_results, filter_result) = cmd_collection.filter(edit_line);
            if (multiple_results)
                printer.format("\n  %\n/%% %%", filter_result, env.volume->get_label(),env.cwd, PROMPT, edit_line);
            else
                suggest_cmd(filter_result);
            break;
        }

        case Key::Enter: {
            printer.newline();
            cmd_history.set_to_latest();
            break;
        }

        case Key::Backspace: {
            if (!edit_line.empty()) {
                edit_line.pop_back();
                printer.backspace();
            }
            break;
        }

        case Key::PgUp: {
            printer.scroll_up(4);
            break;
        }

        case Key::PgDown: {
            printer.scroll_down(4);
            break;
        }

        case Key::Home: {
            printer.scroll_to_begin();
            break;
        }

        case Key::End: {
            printer.scroll_to_end();
            break;
        }

        case Key::Esc:
            break;

        default:;
        }
    }
    else {
        printer.putc(key);
        edit_line.push_back(key);
    }
}

void Terminal::process_cmd(const string& cmd) {
    if (cmd.empty())
        return;

    auto cmd_args = kstd::split_string<vector<string>>(cmd, ' ');
    if (auto task = cmd_collection.get(cmd_args[0])) {
        env.cmd_args = cmd_args;

        TaskManager& task_manager = TaskManager::instance();
        task_manager.add_task(task);
        task->wait_until_finished();
        cmd_history.append(cmd);
    }
    else
        printer.format("Unknown command: %\n", cmd);

    edit_line.clear();
}

void Terminal::print_klog() {
    auto& klog = KernelLog::instance();
    printer.format("%\n", klog.get_text());
}

/**
 * Executed from keyboard interrupt
 */
void Terminal::on_key_press(Key key) {
    last_key = key;
}

} /* namespace terminal */
