/**
 *   @file: Requests.h
 *
 *   @date: Dec 09, 2018
 * @author: Mateusz Midor
 */

#ifndef KERNEL_SERVICES_FILESYSTEM_REQUESTS_H_
#define KERNEL_SERVICES_FILESYSTEM_REQUESTS_H_

#include "StringUtils.h"

// CAREFUL: when changed namespace name,extern Requests* requests; will no longer share
//			pointer from filesystem::requests
namespace filesystem {

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
};

/**
 * @brief	This requests interface is to be assigned with actual Requests implementation
			in the project configuration unit that wires all components together ("main" or similar).
 */
extern Requests* requests;

}

#endif /* KERNEL_SERVICES_FILESYSTEM_REQUESTS_H_ */
