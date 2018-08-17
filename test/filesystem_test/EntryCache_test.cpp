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
    VfsCachedEntryPtr root;
    VfsCachedEntryPtr home;
    VfsCachedEntryPtr images;

    VfsEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsRamDirectoryEntry>(name);
    }

    void SetUp() override {
        root = ec.allocate(mkdir("/"), "/");
        home = ec.allocate(mkdir("HOME"), "/HOME");
        images = ec.allocate(mkdir("images"), "/HOME/images");
    }
};

TEST_F(EntryCacheTest, test_allocate) {
    ASSERT_TRUE(root);
    ASSERT_TRUE(home);
    ASSERT_TRUE(images);
}

TEST_F(EntryCacheTest, test_find) {
    ASSERT_TRUE(ec.find("/"));
    ASSERT_TRUE(ec.find("/HOME"));
    ASSERT_TRUE(ec.find("/HOME/images"));
}

TEST_F(EntryCacheTest, test_deallocate) {
    ec.deallocate(root);
    ASSERT_FALSE(ec.find("/"));
    ec.deallocate(home);
    ASSERT_FALSE(ec.find("/HOME"));
    ec.deallocate(images);
    ASSERT_FALSE(ec.find("/HOME/images"));
}
