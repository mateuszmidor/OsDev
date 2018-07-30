/**
 *   @file: EntryCache_test.cpp
 *
 *   @date: Jul 30, 2018
 * @author: Mateusz Midor
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "EntryCache.h"
#include "VfsRamDirectoryEntry.h"

using namespace filesystem;

class EntryCacheTest : public ::testing::Test {
protected:
    EntryCache ec;
    GlobalFileDescriptor ROOT_FD;

    VfsEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsRamDirectoryEntry>(name);
    }

    void SetUp() override {
        ec.install();
        ROOT_FD = ec.allocate(mkdir("/"), "/").value;
    }
};

TEST_F(EntryCacheTest, test_find) {
    ASSERT_TRUE(ec.find("/"));
}

TEST_F(EntryCacheTest, test_allocate) {
    auto fd = ec.allocate(mkdir("home"), "/");
    ASSERT_TRUE(fd);
}

TEST_F(EntryCacheTest, test_deallocate) {
    ec.deallocate(ec.find("/").value);
    auto fd = ec.find("/");
    ASSERT_FALSE(fd);
}

TEST_F(EntryCacheTest, test_is_in_cache) {
    ASSERT_TRUE(ec.is_in_cache(ROOT_FD));     // 0 for root
    ASSERT_FALSE(ec.is_in_cache(1));    // not allocated fd
}

TEST_F(EntryCacheTest, test_index_accessor) {
    ASSERT_EQ(ec[ROOT_FD]->get_name(), "/");
}
