/**
 *   @file: VfsTree_test.cpp
 *
 *   @date: Jul 30, 2018
 * @author: Mateusz Midor
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "VfsTree.h"
#include "VfsRamDirectoryEntry.h"
#include "VfsRamMountPoint.h"
#include "KernelLog.h"
#include <iostream>

using namespace filesystem;

class VfsTreeTest : public ::testing::Test {
protected:
    VfsTree tree;
    GlobalFileDescriptor ROOT_FD;

    VfsRamDirectoryEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsRamDirectoryEntry>(name);
    }

    void SetUp() override {
        tree.install();
    }
};

TEST_F(VfsTreeTest, test_exists) {
    ASSERT_TRUE(tree.exists("/"));
}

TEST_F(VfsTreeTest, test_attach_to_cached) {
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.exists("/home"));
}

TEST_F(VfsTreeTest, test_create_nomountpoint) {
    ASSERT_FALSE(tree.create("/images", false));
}

TEST_F(VfsTreeTest, test_remove) {
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.exists("/home"));
    ASSERT_TRUE(tree.remove("/home"));
    ASSERT_FALSE(tree.exists("/home"));
}

TEST_F(VfsTreeTest, test_create_multilevel_mountpoint) {
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));

    ASSERT_TRUE(tree.create("/HOME/images/cars", true));
    ASSERT_TRUE(tree.exists("/HOME/images/cars"));

    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("VOLVO"), "/HOME/images/cars"));
    ASSERT_TRUE(tree.exists("/HOME/images/cars/VOLVO"));

    ASSERT_TRUE(tree.create("/HOME/images/cars/VOLVO/2008", true));
    ASSERT_TRUE(tree.exists("/HOME/images/cars/VOLVO/2008"));
}

TEST_F(VfsTreeTest, test_open_existent) {
    ASSERT_TRUE(tree.open("/"));
}

TEST_F(VfsTreeTest, test_open_nonexistent) {
    ASSERT_FALSE(tree.open("/nonexistent"));
}

TEST_F(VfsTreeTest, test_close_opened) {
    auto fd = tree.open("/");
    ASSERT_TRUE(tree.close(fd.value));
}

TEST_F(VfsTreeTest, test_close_nonopened) {
    auto fd = tree.open("/");
    tree.close(fd.value);
    ASSERT_FALSE(tree.close(fd.value));
}

TEST_F(VfsTreeTest, test_open_deeplyhidden) {
    auto home = mkdir("home");
    auto images = mkdir("images");
    auto cars = mkdir("cars");
    auto volvo = mkdir("volvo");
    home->attach_entry(images);
    images->attach_entry(cars);
    cars->attach_entry(volvo);
    tree.attach(home, "/");
    ASSERT_TRUE(tree.open("/home/images/cars/volvo"));
}
