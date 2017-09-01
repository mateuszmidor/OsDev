/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include "GlobalConstructorsRunner.h"
#include "Gdt.h"
#include "KernelLog.h"
#include "Multiboot2.h"
#include "CpuInfo.h"
#include "KeyboardScanCodeSet.h"
#include "InterruptManager.h"
#include "kstd.h"
#include "types.h"
#include "DriverManager.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "PitDriver.h"
#include "SysCallDriver.h"
#include "PCIController.h"
#include "ExceptionManager.h"
#include "VgaDriver.h"
#include "TaskManager.h"
#include "TaskFactory.h"
#include "PageFaultHandler.h"
#include "TaskExitHandler.h"
#include "Terminal.h"
#include "_demos/Demo.h"
#include "_demos/VgaDemo.h"
#include "_demos/Fat32Demo.h"
#include "_demos/MouseDemo.h"
#include "_demos/MultitaskingDemo.h"
#include "_demos/CpuSpeedDemo.h"

using std::make_shared;
using namespace kstd;
using namespace drivers;
using namespace cpuexceptions;
using namespace hardware;
using namespace multitasking;
using namespace utils;
using namespace demos;

Gdt gdt;
PCIController pcic;
KernelLog& klog                     = KernelLog::instance();
TaskManager& task_manager           = TaskManager::instance();
DriverManager& driver_manager       = DriverManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();

KeyboardScanCodeSet1 scs1;
auto keyboard           = make_shared<KeyboardDriver> (scs1);
auto mouse              = make_shared<MouseDriver>();
auto pit                = make_shared<PitDriver>();
auto ata_primary_bus    = make_shared<AtaPrimaryBusDriver>();
auto sys_call           = make_shared<SysCallDriver>();


const char hello_user1[]    = "Hello from user space 1";
const char hello_user2[]    = "Hello from user space 2";
const char hello_kernel[]   = "Hello from kernel space!!!";

void print(u64 arg) {
    static u8 call_number = 0;
    u8 y = call_number;
    call_number++;
    char* s = (char*)arg;
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>()) {
        int x = 0;
        while (*s) {
            vga_drv->at(x, y) = VgaCharacter { *s, EgaColor::Black, EgaColor::White };
            x++;
            s++;
            asm volatile("mov $0, %rax; int $0x80");
        }
    }
}

/**
 * Here we enter multitasking
 */
void task_init(u64 unused) {
    task_manager.add_task(make_shared<Task>(Task::idle, "idle"));
//    task_manager.add_task(Demo::make_demo<MultitaskingDemo>("multitasking_a_demo", 'A'));
//    task_manager.add_task(Demo::make_demo<MultitaskingDemo>("multitasking_b_demo", 'B'));
//    task_manager.add_task(Demo::make_demo<CpuSpeedDemo>("cpuspeed_demo"));
//    task_manager.add_task(Demo::make_demo<Fat32Demo>("fat32_demo"));
    task_manager.add_task(TaskFactory::make<terminal::Terminal>("terminal"));
//    task_manager.add_task(Demo::make_demo<MouseDemo>("mouse_demo"));
//    task_manager.add_task(Demo::make_demo<demos::VgaDemo>("vga_demo"));

    auto print1 = std::make_shared<multitasking::Task>(print, "user_print1", (u64)hello_user1, true);
    auto print2 = std::make_shared<multitasking::Task>(print, "kernel_print", (u64)hello_kernel, false);
    auto print3 = std::make_shared<multitasking::Task>(print, "user_print2", (u64)hello_user2, true);
    task_manager.add_task(print1);
    task_manager.add_task(print2);
    task_manager.add_task(print3);
}

/**
 * kmain
 * Kernel entry point
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // 1. run constructors of global objects. This could be run from long_mode_init.S
    GlobalConstructorsRunner::run();

    // 2. install new Global Descriptor Table that will allow user-mode
    gdt.reinstall_gdt();

    // 3. prepare drivers
    pit->set_channel0_hz(20);
    pit->set_channel0_on_tick([](CpuState* cpu_state) { return task_manager.schedule(cpu_state); });

    // 4. install drivers
    pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);
    driver_manager.install_driver(pit);
    driver_manager.install_driver(ata_primary_bus);
    driver_manager.install_driver(sys_call);

    // 5. install exceptions
    exception_manager.install_handler(make_shared<TaskExitHandler>());
    exception_manager.install_handler(make_shared<PageFaultHandler>());

    // 6. configure interrupt manager
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts();

    // 7. configure vga text mode
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>())
        vga_drv->set_text_mode_90_30();

    // 8. SETUP DONE
    klog.format("KERNEL SETUP DONE.\n");

    // print Multiboot2 info
//    Multiboot2 mb2(multiboot2_info_ptr);
//    mb2.print_to_klog();
//    klog.format("\n");

    // 9. start multitasking
    task_manager.add_task(make_shared<Task>(task_init, "init"));

    // 10. wait until timer interrupt switches execution to init task
    Task::idle();
}
