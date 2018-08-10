/**
 *   @file: OpenEntryTable_test.cpp
 *
 *   @date: Aug 10, 2018
 * @author: Mateusz Midor
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "OpenEntryTable.h"
#include "VfsRamDirectoryEntry.h"

using namespace filesystem;

class OpenEntryTableTest : public ::testing::Test {
protected:
    OpenEntryTable et;

    VfsCachedEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsCachedEntry>(std::make_shared<VfsRamDirectoryEntry>(name));
    }

    void SetUp() override {
        et.install();
    }
};

TEST_F(OpenEntryTableTest, test_open) {
    // setup
    auto entry = mkdir("home");

    // test
    auto fd = et.open(entry);
    ASSERT_TRUE(fd);
    ASSERT_TRUE(et[fd.value].entry);
    ASSERT_TRUE(et[fd.value].state);
}

TEST_F(OpenEntryTableTest, test_is_open) {
    // setup
    auto entry = mkdir("home");
    auto fd = et.open(entry);
    ASSERT_TRUE(fd);

    // test
    ASSERT_TRUE(et.is_open(fd.value));
}

TEST_F(OpenEntryTableTest, test_close) {
    // setup
    auto entry = mkdir("home");
    auto fd = et.open(entry);
    ASSERT_TRUE(fd);

    // test
    ASSERT_TRUE(et.close(fd.value));
    ASSERT_FALSE(et[fd.value].entry);
    ASSERT_FALSE(et[fd.value].state);
}

TEST_F(OpenEntryTableTest, test_close_closed) {
    // setup
    auto entry = mkdir("home");
    auto fd = et.open(entry);
    ASSERT_TRUE(fd);
    ASSERT_TRUE(et.close(fd.value));

    // test
    ASSERT_FALSE(et.close(fd.value));
}

TEST_F(OpenEntryTableTest, test_close_neveropened) {
    ASSERT_FALSE(et.close(7));
}
