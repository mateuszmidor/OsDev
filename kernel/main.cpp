/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "GlobalConstructorsRunner.h"
#include "Gdt.h"
#include "KernelLog.h"
#include "Multiboot2.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "PitDriver.h"
#include "Int80hDriver.h"
#include "PCIController.h"
#include "ExceptionManager.h"
#include "VgaDriver.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageFaultHandler.h"
#include "MemoryManager.h"
#include "BumpAllocationPolicy.h"
#include "WyoosAllocationPolicy.h"
#include "SysCallManager.h"
#include "VfsManager.h"
#include "TimeManager.h"
#include "Sse.h"
#include "PageTables.h"
#include "ElfRunner.h"
#include "Mouse.h"
#include "SysCallNumbers.h"

using namespace kstd;
using namespace logging;
using namespace drivers;
using namespace cpuexceptions;
using namespace hardware;
using namespace multitasking;
using namespace memory;
using namespace syscalls;
using namespace utils;
using namespace filesystem;
using namespace ktime;
using namespace middlespace;

KernelLog& klog                     = KernelLog::instance();
MemoryManager& memory_manager       = MemoryManager::instance();
TaskManager& task_manager           = TaskManager::instance();
DriverManager& driver_manager       = DriverManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();
SysCallManager& syscall_manager     = SysCallManager::instance();
VfsManager& vfs_manager             = VfsManager::instance();
TimeManager& time_manager           = TimeManager::instance();
Gdt                     gdt;
PCIController           pcic;
KeyboardScanCodeSet1    scs1;
KeyboardDriver          keyboard(scs1);
MouseDriver             mouse;
PitDriver               pit;
AtaPrimaryBusDriver     ata_primary_bus;
VgaDriver               vga;
Int80hDriver            int80h;
PageFaultHandler        page_fault;

const u32               PIT_FREQUENCY_HZ = 20;


/**
 * Simple printer for kernel boot time
 */
u32 current_row {0};
u32 current_col {0};
void print(const char s[], EgaColor c = EgaColor::Yellow) {
    vga.print(current_col, current_row, s, c);
    current_col+= strlen(s);
}

void println(const char s[], EgaColor c = EgaColor::Yellow) {
    vga.print(current_col, current_row++, s, c);
    current_col = 0;
}

void setup_temporary_console() {
    vga.set_text_mode_90_30();
    vga.clear_screen();
    vga.set_cursor_visible(false);
}

void print_phobos_loading() {
    println(R"( ____  _           _      ___  ____  )");
    println(R"(|  _ \| |__   ___ | |__  / _ \/ ___| )");
    println(R"(| |_) | '_ \ / _ \| '_ \| | | \___ \ )");
    println(R"(|  __/| | | | (_) | |_) | |_| |___) |)");
    println(R"(|_|   |_| |_|\___/|_.__/ \___/|____/ )");
    println("");
    println("Loading");
}

/**
 * Keyboard handling
 */
VfsEntryPtr keyboard_vfe;
void handle_key_press(const Key &key) {
    if (key != Key::INVALID) {

        // write key to /dev/keyboard RAM file
        if (!keyboard_vfe)
            keyboard_vfe = filesystem::VfsManager::instance().get_entry("/dev/keyboard");

        if (keyboard_vfe) {
            keyboard_vfe->write(&key, sizeof(key));
        }
    }
}

/**
 * Mouse handling
 */
middlespace::MouseState mouse_state;
VfsEntryPtr mouse_vfe;
void update_mouse() {
    // write key to /dev/mouse RAM file
    if (!mouse_vfe)
        mouse_vfe = filesystem::VfsManager::instance().get_entry("/dev/mouse");

    if (mouse_vfe) {
        mouse_vfe->truncate(0);
        mouse_vfe->write(&mouse_state, sizeof(mouse_state));
    }
}

void handle_mouse_down(MouseButton button) {
    mouse_state.buttons[button] = true;
    update_mouse() ;
}

void handle_mouse_up(MouseButton button) {
    mouse_state.buttons[button] = false;
    update_mouse() ;
}

void handle_mouse_move(s8 dx, s8 dy) {
    mouse_state.dx = dx;
    mouse_state.dy = dy;
    update_mouse() ;
}

/**
 * Programmable Interval Timer handling
 */
CpuState* handle_timer_tick(CpuState* cpu_state) {
    time_manager.tick();
    return task_manager.schedule(cpu_state);
}

/**
 *  Little counter in the right-top corner
 */
void corner_counter() {
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>()) {
        u64 i = 0;

        while (true) {
            u8 c = (i % 10) + '0';
            vga_drv->at(vga_drv->screen_width() - 2, 0) = VgaCharacter { c, EgaColor::White, EgaColor::Black };
            i++;
            Task::yield();
        }
    }
}

/**
 * Terminal runner
 */
void run_userspace_terminal() {
    print("  loading terminal");

    auto vga_drv = driver_manager.get_driver<VgaDriver>();
    if (!vga_drv) {
        println("VGA driver not installed. No terminal will be available", EgaColor::Red);
        return;
    }

    VfsEntryPtr e = vfs_manager.get_entry("/BIN/TERMINAL");
    if (!e) {
        println("/BIN/TERMINAL doesnt exist. No terminal will be available", EgaColor::Red);
        return;
    }
    if (e->is_directory()) {
        println("/BIN/TERMINAL is directory? No terminal will be available", EgaColor::Red);
        return;
    }

    // terminal loading animation start
    auto on_expire = [] { print("."); };
    auto timer_id = time_manager.emplace(1000, 1000, on_expire);

    // read elf file data
    u32 size = e->get_size();
    u8* elf_data = new u8[size];
    e->read(elf_data, size);

    // run the elf
    utils::ElfRunner runner;
    if (runner.run(elf_data, new vector<string> { "terminal" }) > 0)
        klog.format("Terminal is running\n");

    // terminal loading animation stop
    time_manager.cancel(timer_id);
}

/**
 * Here we enter multitasking
 */
void task_init() {
    task_manager.add_task(TaskFactory::make_kernel_task(Task::idle, "idle"));
//    task_manager.add_task(TaskFactory::make_kernel_task(corner_counter, "corner_counter"));
    run_userspace_terminal();
}


/**
 * @name    kmain
 * @brief   Kernel entry point, we jump here right from long_mode_init.S
 * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // 0. activate the SSE so the kernel code compiled under -O2 can actually run, remap the kernel to higher half
    Sse::activate_legacy_sse();
    PageTables::map_and_load_kernel_address_space_at_memory_start();

    // 1. initialize multiboot2 info from the data provided by the boot loader
    Multiboot2::initialize(multiboot2_info_ptr);

    // 2. run constructors of global objects
    GlobalConstructorsRunner::run();

    // 3. configure temporary console and print PhobOS
    setup_temporary_console();
    print_phobos_loading();

    // 4. install new Global Descriptor Table that will allow user-space
    gdt.reinstall_gdt();
    println("  installing GDT...done");

    // 5. prepare drivers & install drivers
    time_manager.set_hz(PIT_FREQUENCY_HZ);
    pit.set_channel0_hz(PIT_FREQUENCY_HZ);
    pit.set_channel0_on_tick(handle_timer_tick);
    keyboard.set_on_key_press(handle_key_press);
    mouse.set_on_down(handle_mouse_down);
    mouse.set_on_up(handle_mouse_up);
    mouse.set_on_move(handle_mouse_move);

    // 6. install drivers
    //pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
    driver_manager.install_driver(&keyboard);
    driver_manager.install_driver(&mouse);
    driver_manager.install_driver(&pit);
    driver_manager.install_driver(&ata_primary_bus);
    driver_manager.install_driver(&vga);
    driver_manager.install_driver(&int80h);
    println("  installing drivers...done");

    // 7. install exceptions
    exception_manager.install_handler(&page_fault);    // this guy allows dynamic memory on-page-fault allocation
    println("  installing exceptions...done");

    // 8. configure interrupt manager so that it forwards interrupts and exceptions to proper managers
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts(); // on-page-fault allocation available from here, as page_fault handler installed and activated
    println("  installing interrupts...done");

    // 9. configure dynamic memory management
    MemoryManager::install_allocation_policy<WyoosAllocationPolicy>(Multiboot2::get_available_memory_first_byte(), Multiboot2::get_available_memory_last_byte());
    println("  installing dynamic memory...done");

    // 10. configure and activate system calls through "syscall" instruction
    syscall_manager.config_and_activate_syscalls();
    println("  installing system calls...done");

    // 11. install filesystem root "/"
    vfs_manager.install_root();
    println("  installing Virtual File System...done");

    // 12. start multitasking
    task_manager.add_task(TaskFactory::make_kernel_task(task_init, "init"));
    println("  installing multitasking...done");
    Task::idle();
}
