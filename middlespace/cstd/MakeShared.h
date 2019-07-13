/**
 *   @file: MakeShared.h
 *
 *   @date: Jul 13, 2019
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_MAKESHARED_H_
#define MIDDLESPACE_CSTD_MAKESHARED_H_

#include <memory>


namespace cstd {

/**
 * @brief std::make_shared needing type_info workaround
 */
template <class T, class ...Args>
std::shared_ptr<T> make_shared(Args&&... args) {
	return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#endif /* MIDDLESPACE_CSTD_MAKESHARED_H_ */
