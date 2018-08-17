/**
 *   @file: main.cpp
 *
 *   @date: Jul 30, 2018
 * @author: Mateusz Midor
 */


#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
