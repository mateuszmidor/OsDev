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
#include "CpuInfo.h"
#include <algorithm>

using namespace kstd;
using namespace drivers;
using namespace multitasking;
using namespace utils;

namespace demos {

CommandHistory::CommandHistory() {
    history.push_back("");
}

void CommandHistory::append(const string& cmd) {
    if (!cmd.empty() && cmd != history.back()) {
        history.push_back(cmd);
        set_to_latest();
    }
}

void CommandHistory::set_to_latest() {
    index = history.size();
}

const string& CommandHistory::get_prev() {
    if (index > 0)
        index--;

    return history[index];
}

const string& CommandHistory::get_next() {
    if (index < history.size() -1) {
        index++;
    } else
        index = history.size() -1;

    return history[index];
}

CommandCollection::CommandCollection() {
}

multitasking::Task* CommandCollection::get(const kstd::string& cmd_name) {
    auto filt = [&cmd_name](const Command& cmd) { return cmd.name == cmd_name; };
    auto found = std::find_if(commands.begin(), commands.end(), filt);
    if (found != commands.end())
        return &found->task;
    else
        return nullptr;
}

/**
 * @brief   Match known commands against name patter
 * @param   pattern Command name beginning
 * @return  {false, name} if single command found
 *          {true, name_list} if multiple commands found
 */
std::tuple<bool, string> CommandCollection::filter(const string& pattern) {
    kstd::vector<string> found;
    auto filt = [&pattern](const Command& c) { return c.name.find(pattern) != string::npos; };
    for (const Command& c : commands)
        if (c.name.find(pattern) == 0)
            found.push_back(c.name);

    if (found.empty())
        return std::make_tuple(false, pattern);
    else if (found.size() == 1)
        return std::make_tuple(false, found.back());
    else
        return std::make_tuple(true, kstd::join_string(" ", found));
}

void CommandCollection::install(const string name, multitasking::Task task) {
    commands.push_back(Command{name, task});
}

static void cmd_log(u64 arg) {
    ScrollableScreenPrinter* p = (ScrollableScreenPrinter*)arg;
    KernelLog& klog = KernelLog::instance();
    p->format("%\n", klog.get_text());
}

static void cmd_cpuinfo(u64 arg) {
    ScrollableScreenPrinter* p = (ScrollableScreenPrinter*)arg;
    CpuInfo cpu_info;
    p->format("CPU: % @ %MHz\n", cpu_info.get_vendor(), cpu_info.get_peak_mhz());
}

const string TerminalDemo::PROMPT {"> "};
TerminalDemo::TerminalDemo() :
        printer(0, 0, 89, 29) {

    cmd_collection.install("log", Task::make_kernel_task(cmd_log, "log").set_arg1(&printer));
    cmd_collection.install("cpuinfo", Task::make_kernel_task(cmd_cpuinfo, "cpuinfo").set_arg1(&printer));
}

void TerminalDemo::run(u64 arg) {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        printer.format(PROMPT);

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

void TerminalDemo::suggest_cmd(const string& cmd) {
    for (u16 i = 0; i < edit_line.length(); i++)
        printer.backspace();

    printer.format(cmd);
    edit_line = cmd;
}

void TerminalDemo::process_key(Key key) {
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
                printer.format("\n  %\n%%", filter_result, PROMPT, edit_line);
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

void TerminalDemo::process_cmd(const string& cmd) {
    if (cmd.empty())
        return;

    if (auto task = cmd_collection.get(cmd)) {
        TaskManager& task_manager = TaskManager::instance();
        u32 tid = task_manager.add_task(*task);
        task_manager.wait(tid);
        cmd_history.append(cmd);
    }
    else
        printer.format("Unknown command: %\n", cmd);

    edit_line.clear();
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
