/**
 *   @file: Requests.h
 *
 *   @date: Nov 28, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_CPUEXCEPTIONS_REQUESTS_H_
#define KERNEL_SERVICES_CPUEXCEPTIONS_REQUESTS_H_

#include "StringUtils.h"
#include "CpuState.h"
#include "CommonStructs.h"

namespace cpuexceptions {

/**
 * @brief	This interface allows the component to access the outer world services,
 * 			without creating unnecessary dependencies to the outer world components.
 * 			It is to be implemented in the project configuration unit
 * 			that wires all components together ("main" or similar).
 */
class Requests {
public: // Boilerplate
	virtual ~Requests() = default;
	template<typename ... Args>
	void log(const cstd::string& fmt, Args ... args) { log(cstd::StringUtils::format(fmt, args...)); }

public: // Actual methods to implement
	virtual void log(const cstd::string& s) = 0;
	virtual PageFaultActualReason get_page_fault_reason(u64 faulty_address, u64 pml4_phys_addr, u64 cpu_error_code) = 0;
	virtual bool alloc_missing_page(u64 virtual_address, u64 pml4_phys_addr) = 0;
	virtual cstd::string& get_current_task_name() = 0;
	virtual bool is_current_task_userspace_task() = 0;
	virtual hardware::CpuState* kill_current_task_group() = 0;
};

/**
 * @brief	This requests interface is to be assigned with actual Requests implementation
			in the project configuration unit that wires all components together ("main" or similar).
 */
extern Requests* requests;

}

#endif /* KERNEL_SERVICES_CPUEXCEPTIONS_REQUESTS_H_ */
