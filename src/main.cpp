/**
 *   @file: main.cpp
 *
 *   @date: Jun 2, 2017
 * @author: Mateusz Midor
 */

#include <memory>
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
#include "Terminal.h"
#include "MemoryManager.h"
#include "BumpAllocationPolicy.h"
#include "SysCallManager.h"
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
using namespace memory;
using namespace syscalls;
using namespace utils;
using namespace demos;

Gdt gdt;
PCIController pcic;
MemoryManager& memory_manager       = MemoryManager::instance();
KernelLog& klog                     = KernelLog::instance();
TaskManager& task_manager           = TaskManager::instance();
DriverManager& driver_manager       = DriverManager::instance();
ExceptionManager& exception_manager = ExceptionManager::instance();
InterruptManager& interrupt_manager = InterruptManager::instance();
SysCallManager& syscall_manager     = SysCallManager::instance();
KeyboardScanCodeSet1 scs1;
auto keyboard           = make_shared<KeyboardDriver> (scs1);
auto mouse              = make_shared<MouseDriver>();
auto pit                = make_shared<PitDriver>();
auto ata_primary_bus    = make_shared<AtaPrimaryBusDriver>();
auto sys_call           = make_shared<SysCallDriver>();


/**
 *  Little counter in the right-top corner
 */
void corner_counter(u64 arg) {
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>()) {
        u64 i = 0;

        asm volatile ("syscall" : :"a"(60));  //syscall exit

        while (true) {
            s8 c = (i % 10) + '0';
            vga_drv->at(vga_drv->screen_width() - 2, 0) = VgaCharacter { c, EgaColor::White, EgaColor::Black };
            i++;
            asm volatile("mov $0, %rax; int $0x80");    // yield
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
    task_manager.add_task(TaskFactory::make<terminal::Terminal>("terminal", 0, false));
    task_manager.add_task(Demo::make_demo<MouseDemo>("mouse_demo", 0, true));
    task_manager.add_task(make_shared<multitasking::Task>(corner_counter, "corner_counter", 0, true));
//    task_manager.add_task(Demo::make_demo<demos::VgaDemo>("vga_demo"));
}

/**
 * @brief   Activate the legacy SSE
 * @see     http://developer.amd.com/wordpress/media/2012/10/24593_APM_v21.pdf, 11.3.1  Enabling Legacy SSE Instruction Execution
 */
void activate_legacy_sse() {
    asm volatile (
            // enable FX SAVE/FXRSTOR (CR4 bit 9)
            "mov %%cr4, %%rax                     \n;"
            "or $0x200, %%rax                     \n;"
            "mov %%rax, %%cr4                     \n;"

            // disable emulate coprocessor (CR0 bit 2)
            "mov %%cr0, %%rax                     \n;"
            "and $0xFFFFFFFFFFFFFFFB, %%rax       \n;"
            "mov %%rax, %%cr0                     \n;"

            // enable monitor coprocessor (CR0 bit 1)
            "mov %%cr0, %%rax                     \n;"
            "or $0x2, %%rax                       \n;"
            "mov %%rax, %%cr0                     \n;"
            :
            :
            : "%rax"
    );
}
/**
 * @name    kmain
 * @brief   Kernel entry point, we jump here right from long_mode_init.S
 * @note    We are starting with just stack in place, no dynamic memory available, no global objects constructed yet
 */
extern "C" void kmain(void *multiboot2_info_ptr) {
    // 0. activate the SSE so the kernel code compiled under -O2 can actually run
    activate_legacy_sse();

    // 1. initialize multiboot2 info from the data provided by the boot loader, then setup dynamic memory manager.
    // This must be done before global constructors are run since global constructors may require dynamic memory
    Multiboot2::initialize(multiboot2_info_ptr);
    MemoryManager::install_allocation_policy<BumpAllocationPolicy>(Multiboot2::get_available_memory_first_byte(), Multiboot2::get_available_memory_last_byte());

    // 2. run constructors of global objects
    GlobalConstructorsRunner::run();

    // 3. install new Global Descriptor Table that will allow user-space
    gdt.reinstall_gdt();

    // 4. prepare drivers
    pit->set_channel0_hz(20);
    pit->set_channel0_on_tick([](CpuState* cpu_state) { return task_manager.schedule(cpu_state); });

    // 5. install drivers
    pcic.install_drivers_into(driver_manager);      // if VGA device is present -> VgaDriver will be installed here
    driver_manager.install_driver(keyboard);
    driver_manager.install_driver(mouse);
    driver_manager.install_driver(pit);
    driver_manager.install_driver(ata_primary_bus);
    driver_manager.install_driver(sys_call);

    // 6. install exceptions
    exception_manager.install_handler(make_shared<PageFaultHandler>());

    // 7. configure interrupt manager so that it forwards interrupts and exceptions to proper managers
    interrupt_manager.set_exception_handler([] (u8 exc_no, CpuState *cpu) { return exception_manager.on_exception(exc_no, cpu); } );
    interrupt_manager.set_interrupt_handler([] (u8 int_no, CpuState *cpu) { return driver_manager.on_interrupt(int_no, cpu); } );
    interrupt_manager.config_and_activate_exceptions_and_interrupts();

    // 8. configure and activate system calls through "syscall" instruction
    syscall_manager.config_and_activate_syscalls();

    // 9. configure vga text mode
    if (auto vga_drv = driver_manager.get_driver<VgaDriver>())
        vga_drv->set_text_mode_90_30();

    // 10. start multitasking
    task_manager.add_task(make_shared<Task>(task_init, "init"));
    Task::idle();
}
