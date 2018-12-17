/**
 *   @file: OpenEntry_test.cpp
 *
 *   @date: Aug 17, 2018
 * @author: Mateusz Midor
 */


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "VfsOpenEntry.h"
#include "VfsCachedEntry.h"
#include "VfsRamDummyFileEntry.h"
#include "FilesystemRequests.h"

using namespace filesystem;

class OpenEntryTest : public ::testing::Test {
    FilesystemRequests filesystem_requests;
protected:
    VfsCachedEntryPtr entry;

//    VfsRamDirectoryEntryPtr mkdir(const char name[]) {
//        return std::make_shared<VfsRamDirectoryEntry>(name);
//    }
//
//    std::shared_ptr<VfsRamDummyFileEntry> mkfile(const char name[]) {
//        return std::make_shared<VfsRamDummyFileEntry>(name);
//    }
//
//    std::shared_ptr<VfsRamMountPoint> mkmountpoint(const char name[]) {
//        return std::make_shared<VfsRamMountPoint>(name);
//    }

    void SetUp() override {
        filesystem::requests = &filesystem_requests;
        entry = std::make_shared<VfsCachedEntry>(std::make_shared<VfsRamDummyFileEntry>("picture.jpg"));
    }

};


TEST_F(OpenEntryTest, test_constructor_increments_open_count) {
    // setup
    ASSERT_EQ(entry->open_count, 0);

    // test
    VfsOpenEntry open1(entry);
    ASSERT_EQ(entry->open_count, 1);

    VfsOpenEntry open2(entry);
    ASSERT_EQ(entry->open_count, 2);
}

TEST_F(OpenEntryTest, test_destructor_decrements_open_count) {
    // setup
    ASSERT_EQ(entry->open_count, 0);
    VfsOpenEntry open1(entry);
    VfsOpenEntry open2(entry);
    ASSERT_EQ(entry->open_count, 2);

    // test
    open1.~VfsOpenEntry();
    ASSERT_EQ(entry->open_count, 1);

    open2.~VfsOpenEntry();
    ASSERT_EQ(entry->open_count, 0);
}

TEST_F(OpenEntryTest, test_assignment_decrements_open_count) {
    // setup
    ASSERT_EQ(entry->open_count, 0);
    VfsOpenEntry open1(entry);
    VfsOpenEntry open2(entry);
    ASSERT_EQ(entry->open_count, 2);

    // test
    open1 = {};
    ASSERT_EQ(entry->open_count, 1);

    open2 = {};
    ASSERT_EQ(entry->open_count, 0);
}
