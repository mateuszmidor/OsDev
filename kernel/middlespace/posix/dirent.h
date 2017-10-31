/**
 *   @file: dirent.h
 *
 *   @date: Oct 21, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_MIDDLESPACE_POSIX_DIRENT_H_
#define SRC_MIDDLESPACE_POSIX_DIRENT_H_

enum dirent_type {
//    DT_UNKNOWN  = 0,
//    DT_FIFO     = 1,
//    DT_CHR      = 2,
    DT_DIR      = 4,        // directory
//    DT_BLK      = 6,
    DT_REG      = 8,        // file
//    DT_LNK      = 10,
//    DT_SOCK     = 12,
//    DT_WHT      = 14
};

typedef struct DIR {};

struct dirent {
    unsigned long long  d_ino;
    unsigned long long  d_off;
    unsigned short int  d_reclen;
    char                d_name[256];
    unsigned char       d_type;         // dirent_type
};


#endif /* SRC_MIDDLESPACE_POSIX_DIRENT_H_ */
