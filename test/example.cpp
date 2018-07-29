/**
 *   @file: example.cpp
 *
 *   @date: Jul 29, 2018
 * @author: Mateusz Midor
 */


#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(EXAMPLE, test_true) {
    ASSERT_TRUE(true);
}

TEST(EXAMPLE, test_false) {
    ASSERT_TRUE(false);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


