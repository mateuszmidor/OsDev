/**
 *   @file: Requests.h
 *
 *   @date: Nov 28, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_MULTITASKING_REQUESTS_H_
#define KERNEL_SERVICES_MULTITASKING_REQUESTS_H_

#include <functional>
#include "StringUtils.h"

namespace multitasking {

/**
 * @brief	This interface allows the component to access the outer world services,
 * 			without creating unnecessary dependencies to the outer world components.
 * 			It is to be implemented in the project configuration unit
 * 			that wires all components together ("main" or similar).
 */
class Requests {
public:
	using OnTimerExpire = std::function<void()>;

public: // Boilerplate
	virtual ~Requests() = default;
	template<typename ... Args>
	void log(const cstd::string& fmt, Args ... args) { log(cstd::StringUtils::format(fmt, args...)); }

public: // Actual methods to implement
	virtual void log(const cstd::string& s) = 0;
	virtual void timer_emplace(u32 millis, const OnTimerExpire& on_expire) = 0;
};

/**
 * @brief	This requests interface is to be assigned with actual Requests implementation
			in the project configuration unit that wires all components together ("main" or similar).
 */
extern Requests* requests;

}

#endif /* KERNEL_SERVICES_MULTITASKING_REQUESTS_H_ */
