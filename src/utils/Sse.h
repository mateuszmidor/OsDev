/**
 *   @file: Sse.h
 *
 *   @date: Sep 15, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_SSE_H_
#define SRC_UTILS_SSE_H_


namespace utils {

/**
 * @brief   This class manages CPU state related to SSE multimedia extensions activation
 */
class Sse {
public:
    static bool activate_legacy_sse();
};

} /* namespace utils */

#endif /* SRC_UTILS_SSE_H_ */
