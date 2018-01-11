/**
 *   @file: Terminal.cpp
 *
 *   @date: Jul 27, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "Terminal.h"
#include "StringUtils.h"
#include "Cin.h"
#include "syscalls.h"

#include "cmds/cd.h"
#include "cmds/cls.h"
#include "cmds/specificelfrun.h"


using namespace ustd;
using namespace cmds;
using namespace middlespace;

namespace terminal {


Terminal::Terminal(const ustd::string& terminal_binary_name) : self_binary_name(terminal_binary_name), user_input(printer) {
}

/**
 * @brief   Run the REPL (Read, Eval, Print, Loop)
 */
void Terminal::run() {
    if (!init())
        return;

    // task main loop
    while(true) {
        // print prompt
        user_input.prompt();

        // get command
        string cmd = cin::readln();

        // process command
        process_cmd(cmd);
    };

    // terminal never exits so no need to wait for its slave threads
}

bool Terminal::init() {
    fd_keyboard = syscalls::open("/dev/keyboard");
    if (fd_keyboard < 0) {
        return false;
    }

    fd_stdin = syscalls::open("/dev/stdin");
    if (fd_stdin < 0) {
        return false;
    }

    fd_stdout = syscalls::open("/dev/stdout");
    if (fd_stdout < 0) {
        return false;
    }

    // go /HOME
    syscalls::chdir("/HOME");

    // setup screen printer and print welcome message
    setup_screen_and_print_welcome();

    // install internal commands
    install_internal_command(new cmds::cd, "cd");
    install_internal_command(new cmds::cls(printer), "cls");

    // install external commands that are in form of separate ELF programs, located under given directory
    install_external_commands("/BIN", self_binary_name);

    // start a thread reading /dev/keyboard
    syscalls::task_lightweight_run((unsigned long long)key_processor_thread, (unsigned long long)this, "terminal_key_processor");

    // start a thread printing /dev/stdout to the screen
    syscalls::task_lightweight_run((unsigned long long)stdout_printer_thread, (unsigned long long)this, "terminal_stdout_printer");

    return true;
}

/**
 * @brief   Setup screen printing for current resolution, print welcome message
 */
void Terminal::setup_screen_and_print_welcome() {
    // setup
    u16 vga_width;
    u16 vga_height;
    syscalls::vga_get_width_height(&vga_width, &vga_height);
    printer.reset(std::make_shared<ScrollableScreenPrinter>(0, 0, vga_width - 1, vga_height - 1));

    // print
    string line(vga_width - 1, '=');
    auto p = printer.get();
    p->format(line);
    p->format("Welcome to PhobOS Terminal.\nUse <TAB> for autocompletion, '&' for running tasks in background\n");
    p->format(line);
    p->format("\n\n");
}

/**
 * @brief   Install command "cmd" under name "cmd_name"
 */
void Terminal::install_internal_command(cmds::CmdBase* cmd, const string& cmd_name) {
    user_input.install(cmd_name);
    cmd_collection.install(cmd_name, cmd);
}

/**
 * @brief   Install commands that are exteral ELF files located in filesystem under "dir"
 */
void Terminal::install_external_commands(const string& dir, const ustd::string& omit_name) {
    int fd = syscalls::open(dir.c_str());

    // no such directory
    if (fd < 0) {
        printer.get()->format("Terminal: cant open directory %\n", dir);
        return;
    }

    // get "dir" contents
    std::array<VfsEntry, 128> entries;
    int count = syscalls::enumerate(fd, entries.data(), entries.size());
    syscalls::close(fd);

    // install found commands except for omit_name
    for (int i = 0; i < count; i++) {
        VfsEntry& e = entries[i];

        // dont install directory. Should also check if it is actually an ELF file...
        if (e.is_directory)
            continue;

        string lowercase_name = StringUtils::to_lower_case(e.name);
        if (lowercase_name == StringUtils::to_lower_case(omit_name))
            continue;

        string cmd_absolute_path = StringUtils::format("%/%", dir, e.name);
        install_internal_command(new cmds::specificelfrun(cmd_absolute_path), lowercase_name);
    }
}

/**
 * @brief   User entered a command. Do something with it
 */
void Terminal::process_cmd(const string& cmd) {
    if (cmd.empty())
        return;

    auto cmd_args = ustd::StringUtils::split_string(cmd, ' ');
    bool run_in_background = false;
    if (cmd_args.back() == "&") {
        run_in_background = true;
        cmd_args.pop_back();
    }

    if (CmdBase* task = cmd_collection.get(cmd_args[0])) {
        task->run(cmd_args, run_in_background);
        cmd_history.append(cmd);
        cmd_history.set_to_latest();
    }
    else
        printer.get()->format("Unknown command: '%'\n", cmd);
}

/**
 * @brief   Handle key down event
 */
void Terminal::on_key_down(Key key) {
    // functional key
    if (key & Key::FUNCTIONAL) {
        switch (key) {
        case Key::Up: {
            user_input.suggest_cmd(cmd_history.get_prev());
            break;
        }

        case Key::Down: {
            user_input.suggest_cmd(cmd_history.get_next());
            break;
        }

        case Key::Tab: {
            user_input.help_me_out();
            break;
        }

        case Key::Backspace: {
            user_input.backspace();
            break;
        }

        case Key::PgUp: {
            printer.get()->scroll_up(4);
            break;
        }

        case Key::PgDown: {
            printer.get()->scroll_down(4);
            break;
        }

        case Key::Home: {
            printer.get()->scroll_to_begin();
            break;
        }

        case Key::End: {
            printer.get()->scroll_to_end();
            break;
        }

        case Key::Esc:
            break;

        case Key::Enter: {
            user_input.putc('\n');
            syscalls::write(fd_stdin, user_input.get_text().c_str(), user_input.get_text().length()); // this makes cin::readln() work :)
            user_input.clear();
            break;
        }

        default:;
        }
    }
    // just an ASCII character
    else {
        user_input.putc(key);
    }
}

/**
 * @brief   This function reads /dev/stdout in a loop and prints it to the screen
 * @note    This function is run in a separate thread
 */
void Terminal::stdout_printer_thread(Terminal* term) {
    const u32 BUFF_SIZE = 512;
    char buff[BUFF_SIZE];

    while (true) {
        ssize_t count = syscalls::read(term->fd_stdout, buff, BUFF_SIZE - 1);
        buff[count] = '\0';
        term->printer.get()->format("%", buff);
    }
}

/**
 * @brief   This function reads /dev/keyboard and runs Terminal::on_key_down
 * @note    This function is run in a separate thread
 */
void Terminal::key_processor_thread(Terminal* term) {
    Key key;

    while (true) {
        syscalls::read(term->fd_keyboard, &key, sizeof(key));
        term->on_key_down(key);
    }
}

} /* namespace terminal */
