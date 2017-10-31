/**
 *   @file: utils.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_UTILS_H_
#define ELFS_UTILS_H_

#include <errno.h>
#include <string.h>
#include "syscalls.h"


int stdout_fd = 0;
size_t bump_addr = 128*1024*1024;

void* umalloc(size_t size) {
    auto old_bump_addr = bump_addr;
    bump_addr+= size;
    return (void*)old_bump_addr;
}

void ufree(void* address) {
}


void* operator new(size_t size) {
    return umalloc(size);
}

void* operator new[](size_t size) {
    return umalloc(size);
}

void operator delete[](void* ptr) noexcept {
    ufree(ptr);
}

void operator delete(void *ptr) noexcept {
    ufree(ptr);
}

extern "C" int memcmp ( const void * ptr1, const void * ptr2, size_t num ) {
    const char *p1 = (const char*)ptr1;
    const char *p2 = (const char*)ptr2;
    for (size_t i = 0; i < num; i++)
        if (p1[i] < p2[i])
            return -1;
        else if (p1[i] > p2[i])
            return 1;

    return 0;
}

/**
 * @brief   Convert string to long
 */
long long str_to_long(const char* str) {
    bool negative = false;
    if (str[0] == '-') {
        str++;
        negative = true;
    }

    long long res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    for (int i = 0; str[i] != '\0'; ++i) {
        unsigned char c = str[i];
        res = res*10 + c - '0';
    }

    return negative ? -res : res;
}

void long_to_str(long long num, unsigned char base, char buff_12_chars[12]) {
    for (int i = 0; i < 12; i++)
        buff_12_chars[i] = '\0';

    unsigned char i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) {
        buff_12_chars[0] = '0';
        return;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        buff_12_chars[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        buff_12_chars[i++] = '-';

    buff_12_chars[i] = '\0'; // Append string terminator

    // Reverse the string
    unsigned char start = 0;
    unsigned char end = i -1;
    while (start < end) {
        auto tmp = *(buff_12_chars+start);
        *(buff_12_chars+start) = *(buff_12_chars+end);
        *(buff_12_chars+end) = tmp;
        start++;
        end--;
    }
}

void _print(int fd, const char str[], size_t len) {
    while (len > 0) {
        ssize_t written = syscalls::write(fd, str, len);
        if (written == -EWOULDBLOCK)
            syscalls::usleep(0);
        else if (written < 0)
            break;
        else {
            str += written;
            len -= written;
        }
    }
}

void print(const char str[], size_t count) {
    if (stdout_fd == 0)
        stdout_fd = syscalls::open("/dev/stdout", 2);
    _print(stdout_fd, str, count);
}

void print(const char str[]) {
   print(str, strlen(str));
}


#endif /* ELFS_UTILS_H_ */
