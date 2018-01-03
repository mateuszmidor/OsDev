/**
 *   @file: Optional.h
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_OPTIONAL_H_
#define USER_USTD_SRC_OPTIONAL_H_

#include "String.h"

namespace ustd {

// This Optional doesnt support "string" as "string" constructor indicates error value
template <class T>
class Optional {
public:
    Optional() : invalid(true) {}
    Optional(const string& error) : error_msg(error), invalid(true) {}
    Optional(const T& value) : value(value), invalid(false) {}
    Optional(T&& value) : value(std::move(value)), invalid(false) {}
    operator bool() const { return !invalid; }
    bool operator!() const { return invalid; }

    string  error_msg {};
    T       value {};

private:
    bool    invalid;
};

// forbidden
template <>
class Optional<string> {
};

} /* namespace ustd */

#endif /* USER_USTD_SRC_OPTIONAL_H_ */
