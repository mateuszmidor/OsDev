/**
 *   @file: ErrorCode.h
 *
 *   @date: Feb 14, 2018
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_ERRORCODE_H_
#define MIDDLESPACE_ERRORCODE_H_

namespace middlespace {

/**
 * @brief   errno.h equivalent error codes for system call interface
 */
enum class ErrorCode {
   EC_OK          = 0,
   EC_PERM        = 1,  /* Operation not permitted */
   EC_NOENT       = 2,  /* No such file or directory */
   EC_SRCH        = 3,  /* No such process */
   EC_INTR        = 4,  /* Interrupted system call */
   EC_IO          = 5,  /* I/O error */
   EC_NXIO        = 6,  /* No such device or address */
   EC_2BIG        = 7,  /* Argument list too long */
   EC_NOEXEC      = 8,  /* Exec format error */
   EC_BADF        = 9,  /* Bad file number */
   EC_CHILD      = 10,  /* No child processes */
   EC_AGAIN      = 11,  /* Try again */
   EC_NOMEM      = 12,  /* Out of memory */
   EC_ACCES      = 13,  /* Permission denied */
   EC_FAULT      = 14,  /* Bad address */
   EC_NOTBLK     = 15,  /* Block device required */
   EC_BUSY       = 16,  /* Device or resource busy */
   EC_EXIST      = 17,  /* File exists */
   EC_XDEV       = 18,  /* Cross-device link */
   EC_NODEV      = 19,  /* No such device */
   EC_NOTDIR     = 20,  /* Not a directory */
   EC_ISDIR      = 21,  /* Is a directory */
   EC_INVAL      = 22,  /* Invalid argument */
   EC_NFILE      = 23,  /* File table overflow */
   EC_MFILE      = 24,  /* Too many open files */
   EC_NOTTY      = 25,  /* Not a typewriter */
   EC_TXTBSY     = 26,  /* Text file busy */
   EC_FBIG       = 27,  /* File too large */
   EC_NOSPC      = 28,  /* No space left on device */
   EC_SPIPE      = 29,  /* Illegal seek */
   EC_ROFS       = 30,  /* Read-only file system */
   EC_MLINK      = 31,  /* Too many links */
   EC_PIPE       = 32,  /* Broken pipe */
   EC_DOM        = 33,  /* Math argument out of domain of func */
   EC_RANGE      = 34,  /* Math result not representable */
};

} /* namespace middlespace */

#endif /* MIDDLESPACE_ERRORCODE_H_ */
