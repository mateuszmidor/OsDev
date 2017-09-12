/**
 *   @file: utils.h
 *
 *   @date: Sep 12, 2017
 * @author: Mateusz Midor
 */

#ifndef ELFS_UTILS_H_
#define ELFS_UTILS_H_

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



#endif /* ELFS_UTILS_H_ */
