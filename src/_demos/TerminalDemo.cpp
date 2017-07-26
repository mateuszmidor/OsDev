/*
 * TerminalDemo.cpp
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#include "TerminalDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "TaskManager.h"

using namespace kstd;
using namespace drivers;
using namespace multitasking;
using utils::KernelLog;

namespace demos {

CommandHistory::CommandHistory() {
    history.push_back("");
}

void CommandHistory::append(const kstd::string& cmd) {
    if (!cmd.empty() && cmd != history.back()) {
        history.push_back(cmd);
        set_to_latest();
    }
}

void CommandHistory::set_to_latest() {
    index = history.size();
}

const kstd::string& CommandHistory::get_prev() {
    if (index > 0)
        index--;

    return history[index];
}

const kstd::string& CommandHistory::get_next() {
    if (index < history.size() -1) {
        index++;
    } else
        index = history.size() -1;

    return history[index];
}

TerminalDemo::TerminalDemo() :
        printer(0, 0, 89, 29) {
}

void TerminalDemo::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        printer.format("> ");

        // get command
        string cmd = get_line();

        // process command
        process_cmd(cmd);
    };
}

bool TerminalDemo::init() {
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

string TerminalDemo::get_line() {
    Key key;
    do  {
        key = get_key();
        process_key(key);
    } while (key != Key::Enter);

    return edit_line;
}

Key TerminalDemo::get_key() {
    // wait until last_key is set by keyboard interrupt. Keys might be missed if stroking faster than TerminalDemo gets CPU time :)
    while (last_key == Key::INVALID)
        Task::yield();

    Key key = last_key;
    last_key = Key::INVALID;
    return key;
}

void TerminalDemo::suggest_cmd(const kstd::string& cmd) {
    for (u16 i = 0; i < edit_line.length(); i++)
        printer.backspace();

    printer.format("%", cmd);
    printer.scroll_to_end();
    edit_line = cmd;
}

void TerminalDemo::process_key(Key key) {
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up:
            suggest_cmd(cmd_history.get_prev());
            break;

        case Key::Down:
            suggest_cmd(cmd_history.get_next());
            break;

        case Key::Enter:
            printer.newline();
            cmd_history.set_to_latest();
            break;

        case Key::Backspace:
            if (!edit_line.empty()) {
                edit_line.pop_back();
                printer.backspace();
            }
            printer.scroll_to_end();
            break;

        case Key::PgUp:
            printer.scroll_up(4);
            break;

        case Key::PgDown:
            printer.scroll_down(4);
            break;

        case Key::Home:
            printer.scroll_to_begin();
            break;

        case Key::End:
            printer.scroll_to_end();
            break;

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

void TerminalDemo::process_cmd(const kstd::string& cmd) {
    if (cmd == "exit") {
        printer.format("Terminal exit.");
        printer.set_cursor_visible(false);
        Task::exit();
    }
    else if (cmd == "log")
        print_klog();
    else if (!cmd.empty())
        printer.format("Unknown command: %\n", cmd);

    cmd_history.append(cmd);
    edit_line.clear();
    printer.scroll_to_end();
}

void TerminalDemo::print_klog() {
    auto& klog = KernelLog::instance();
    printer.format("%\n", klog.get_text());
}

/**
 * Executed from keyboard interrupt
 */
void TerminalDemo::on_key_press(Key key) {
    last_key = key;
}

} /* namespace demos */
