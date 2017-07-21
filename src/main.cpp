/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "GlobalConstructorsRunner.h"
#include "KernelLog.h"
#include "ScreenPrinter.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "TimerDriver.h"
#include "PCIController.h"
#include "ExceptionManager.h"
#include "VgaDriver.h"
#include "TaskManager.h"
#include "PageFaultHandler.h"
#include "TaskExitHandler.h"

#include "_demos/VgaDemo.h"
#include "_demos/Fat32Demo.h"
#include "_demos/MouseDemo.h"
#include "_demos/TerminalDemo.h"

using std::make_shared;
using namespace kstd;
using namespace drivers;
using namespace cpuexceptions;
using namespace cpu;


KernelLog& klog = KernelLog::instance();
ScreenPrinter& printer = ScreenPrinter::instance();
TaskManager& task_manager = TaskManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();
DriverManager& driver_manager = DriverManager::instance();

KeyboardScanCodeSet1 scs1;
auto keyboard = make_shared<KeyboardDriver> (scs1);
auto mouse = make_shared<MouseDriver>();
auto timer = make_shared<TimerDriver>();
auto ata_primary_bus = make_shared<AtaPrimaryBusDriver>();



CpuState* on_timer_tick(CpuState* cpu_state) {
    return task_manager.schedule(cpu_state);
}

// at least this guy should ever live :)
void task_idle() {
    while (true)
        asm("hlt");
}

template <class T>
void make_demo_() {
    T demo;
    demo.run();
    while (true)
        asm("hlt");
}

template <class T>
std::shared_ptr<Task> make_demo(string name) {
    return make_shared<Task>(make_demo_<T>, name);
}

void task_init() {
    task_manager.add_task(make_shared<Task>(task_idle, "idle"));
    task_manager.add_task(make_demo<demos::Fat32Demo>("fat32_demo"));
    task_manager.add_task(make_demo<demos::TerminalDemo>("terminal_demo"));
    task_manager.add_task(make_demo<demos::MouseDemo>("mouse_demo"));
//    task_manager.add_task(make_shared<Task>(vga_demo, "vga_demo"));
//    task_manager.add_task(make_shared<Task>(task_print_A, "A printer"));
//    task_manager.add_task(make_shared<Task>(task_print_b, "B printer"));
}

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // run constructors of global objects. This could be run from long_mode_init.S
    GlobalConstructorsRunner::run();

    // configure drivers
    timer->set_on_tick(on_timer_tick);

    // install drivers
    PCIController pcic;
    pcic.install_drivers_into(driver_manager);
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);
    driver_manager.install_driver(timer);
    driver_manager.install_driver(ata_primary_bus);

    // install exceptions
    exception_manager.install_handler(make_shared<TaskExitHandler>());
    exception_manager.install_handler(make_shared<PageFaultHandler>());

    // configure Interrupt Descriptor Table
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts();

    // prepare the text  mode
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>())
        vga_drv->set_text_mode_90_30();

    printer.clearscreen();
    printer.set_bg_color(EgaColor::Blue);
    klog.printer.clear_screen();



    // print CPU info
    CpuInfo cpu_info;
    cpu_info.print_to_klog();
    klog.format("\n");

    // print Multiboot2 info related to framebuffer config, available memory and kernel ELF sections
    Multiboot2 mb2(multiboot2_info_ptr);
    mb2.print_to_klog();
    klog.format("\n");

    // print PCI devics
    pcic.drivers_to_klog();
    klog.format("\n");

    // inform setup done
    klog.format("KERNEL SETUP DONE.\n");


    // start multitasking
    task_manager.add_task(make_shared<Task>(task_init, "init"));

    
    // wait until timer interrupt switches execution to init task
    while (true)
        asm("hlt");
}
