/**
 *   @file: utils.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef USER_USTD_SRC_UTILS_H_
#define USER_USTD_SRC_UTILS_H_

#include <errno.h>
#include "syscalls.h"
#include "Vector.h"
#include "String.h"

void _print(int fd, const char str[], size_t len);

void print(const char str[], size_t count);

void print(const char str[]);


#endif /* USER_USTD_SRC_UTILS_H_ */
