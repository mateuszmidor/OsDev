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
    VfsCachedEntryPtr ROOT;

    VfsEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsRamDirectoryEntry>(name);
    }

    void SetUp() override {
        ROOT = ec.allocate(mkdir("/"), "/");
    }
};

TEST_F(EntryCacheTest, test_find) {
    ASSERT_TRUE(ec.find("/"));
}

TEST_F(EntryCacheTest, test_deallocate) {
    ASSERT_TRUE(ec.find("/"));
    ec.deallocate(ROOT);
    ASSERT_FALSE(ec.find("/"));
}
