/**
 *   @file: VfsManager_test.cpp
 *
 *   @date: Aug 17, 2018
 * @author: Mateusz Midor
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include "VfsManager.h"
#include "VfsRamDirectoryEntry.h"
#include "VfsRamDummyFileEntry.h"
#include "VfsRamMountPoint.h"
#include "FilesystemRequests.h"

using namespace filesystem;

class VfsManagerTest : public ::testing::Test {
    FilesystemRequests filesystem_requests;
protected:
    VfsManager& manager = VfsManager::instance();

    void SetUp() override {
        filesystem::requests = &filesystem_requests;
        manager.install();
    }
};

/**************************************************************************
 * VfsManager::exists
 *************************************************************************/
TEST_F(VfsManagerTest, test_exists_basic) {
    // setup
    // root already in place

    // test
    ASSERT_TRUE(manager.exists("/"));
}

TEST_F(VfsManagerTest, test_exists_nonexistent) {
    // setup
    // root already in place

    // test
    ASSERT_FALSE(manager.exists("/HOME"));
}

/**************************************************************************
 * VfsManager::open
 *************************************************************************/
TEST_F(VfsManagerTest, test_open_existent) {
    // setup
    // root already in place

    // test
    auto open = manager.open("/");
    ASSERT_TRUE(open);
}

TEST_F(VfsManagerTest, test_open_nonexistent) {
    // setup
    // root already in place

    // test
    auto open = manager.open("/HOME");
    ASSERT_EQ(open.ec, middlespace::ErrorCode::EC_NOENT);
}
