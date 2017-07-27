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
#include <algorithm>
#include <memory>

using namespace kstd;
using namespace utils;
using namespace drivers;
using namespace multitasking;

namespace terminal {

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

static void cmd_df(u64 arg) {
    ScrollableScreenPrinter* p = (ScrollableScreenPrinter*)arg;
//    p->format("CPU: % @ %MHz\n", cpu_info.get_vendor(), cpu_info.get_peak_mhz());
}

const string Terminal::PROMPT {"> "};
Terminal::Terminal() :
        printer(0, 0, 89, 29) {
    cmd_collection.install("log", std::make_shared<Task>(cmd_log, "log", (u64)&printer));
    cmd_collection.install("cpuinfo", std::make_shared<Task>(cmd_cpuinfo, "cpuinfo", (u64)&printer));
    cmd_collection.install("df", std::make_shared<Task>(cmd_df, "df", (u64)&printer));
}

void Terminal::run(u64 arg) {
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

void Terminal::process_cmd(const string& cmd) {
    if (cmd.empty())
        return;

    if (auto task = cmd_collection.get(cmd)) {
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
