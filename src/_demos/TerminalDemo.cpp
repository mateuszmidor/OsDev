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

void TerminalDemo::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // get command
        printer.format("> ");
        string cmd = get_line();

        // process command
        process_cmd(cmd);

        // move to next line
        printer.format("\n");
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
    cmd_history.push_back("");
    return true;
}

string TerminalDemo::get_line() {
    Key key;
    do  {
        key = get_key();
        process_key(key, line);
    } while (key != Key::Enter);

    return line;
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
    if (!line.empty())
        for (u8 i = 0; i < line.length(); i++)
            printer.format("\x08");

    line = cmd;
    printer.format("%", cmd);
}

void TerminalDemo::process_key(Key key, string& current_line) {
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up:
           // if (cmd_history_index < cmd_history.size())
            if (cmd_history_index > 0)
                cmd_history_index--;
            suggest_cmd(cmd_history[cmd_history_index]);
            break;

        case Key::Down:
            if (cmd_history_index < cmd_history.size() -1) {
                cmd_history_index++;
            } else
                cmd_history_index = cmd_history.size() -1;
            suggest_cmd(cmd_history[cmd_history_index]);
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
            printer.clear_screen();
            break;

        case Key::Enter:
            printer.format("\n");
            break;

        case Key::Backspace:
            if (!current_line.empty()) {
                printer.format("\x08");
                current_line.pop_back();
            }
            break;

        default:;
        }
    }
    else {
        char s[2] = {(char)key, '\0'};
        printer.format("%", s);
        current_line += (char)key;
        //printer.scroll_to_end();
    }
}

void TerminalDemo::process_cmd(const kstd::string& cmd) {
    if (cmd == "exit") {
        printer.format("Terminal exit.");
        Task::exit();
    }
    else if (cmd == "log")
        print_klog();
    else if (!cmd.empty())
        printer.format("Unknown command: %", cmd);
    else
        printer.scroll_to_end();

    if (!cmd.empty() && cmd != cmd_history.back()) {
        cmd_history.push_back(cmd);
        cmd_history_index = cmd_history.size();
    }
    line.clear();
}

void TerminalDemo::print_klog() {
    auto& klog = KernelLog::instance();
    printer.format("%", klog.get_text());
}

/**
 * Executed from keyboard interrupt
 */
void TerminalDemo::on_key_press(Key key) {
    last_key = key;
}

} /* namespace demos */
