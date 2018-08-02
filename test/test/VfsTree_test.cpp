/**
 *   @file: VfsTree_test.cpp
 *
 *   @date: Jul 30, 2018
 * @author: Mateusz Midor
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include "VfsTree.h"
#include "VfsRamDirectoryEntry.h"
#include "VfsRamMountPoint.h"
#include "KernelLog.h"

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
        logging::KernelLog::instance().clear();
    }

    void print_log() {
        std::cout << logging::KernelLog::instance().get_text() << std::endl;
    }
};

/**************************************************************************
 * VfsTree::exists
 *************************************************************************/
TEST_F(VfsTreeTest, test_exists_basic) {
    // setup
    // root already in place

    // test
    ASSERT_TRUE(tree.exists("/"));
}

TEST_F(VfsTreeTest, test_exists_deeplyhidden) {
    // setup
    auto home = mkdir("home");
    auto images = mkdir("images");
    auto cars = mkdir("cars");
    auto volvo = mkdir("volvo");
    home->attach_entry(images);
    images->attach_entry(cars);
    cars->attach_entry(volvo);
    tree.attach(home, "/");

    // test
    ASSERT_TRUE(tree.exists("/home/images/cars/volvo"));
}

/**************************************************************************
 * VfsTree::attach
 *************************************************************************/
TEST_F(VfsTreeTest, test_attach_basic) {
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.exists("/home"));
}

TEST_F(VfsTreeTest, test_attach_in_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    // test
    ASSERT_TRUE(tree.attach(mkdir("images"), "/HOME"));
    ASSERT_TRUE(tree.exists("/HOME/images"));
}

TEST_F(VfsTreeTest, test_attach_in_multilevel_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));

    ASSERT_TRUE(tree.create("/HOME/images/cars", true));
    ASSERT_TRUE(tree.exists("/HOME/images/cars"));

    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("VOLVO"), "/HOME/images/cars"));
    ASSERT_TRUE(tree.exists("/HOME/images/cars/VOLVO"));

    // test
    ASSERT_TRUE(tree.attach(mkdir("2008"), "/HOME/images/cars/VOLVO/"));
    ASSERT_TRUE(tree.exists("/HOME/images/cars/VOLVO/2008"));
}

/**************************************************************************
 * VfsTree::create
 *************************************************************************/
TEST_F(VfsTreeTest, test_create_without_mountpoint) {
    ASSERT_FALSE(tree.create("/images", false));
    ASSERT_FALSE(tree.exists("/images"));
}

TEST_F(VfsTreeTest, test_create_in_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    // test
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));
}

TEST_F(VfsTreeTest, test_create_in_multilevel_mountpoint) {
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

/**************************************************************************
 * VfsTree::remove
 *************************************************************************/
TEST_F(VfsTreeTest, test_remove_attached) {
    // setup
    tree.attach(mkdir("HOME"), "/");
    ASSERT_TRUE(tree.exists("/HOME"));

    tree.attach(mkdir("images"), "/HOME");
    ASSERT_TRUE(tree.exists("/HOME/images"));

    // test
    tree.remove("/HOME/images");
    ASSERT_FALSE(tree.exists("/HOME/images"));

    tree.remove("/HOME");
    ASSERT_FALSE(tree.exists("/HOME"));
}

TEST_F(VfsTreeTest, test_remove_in_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    auto fd = tree.create("/HOME/images", true);
    ASSERT_TRUE(fd);
    ASSERT_TRUE(tree.exists("/HOME/images"));
    ASSERT_TRUE(tree.close(fd.value));

    // test
    ASSERT_TRUE(tree.remove("/HOME/images"));
    ASSERT_FALSE(tree.exists("/HOME/images"));
}

/**************************************************************************
 * VfsTree::open
 *************************************************************************/
TEST_F(VfsTreeTest, test_open_existent) {
    ASSERT_TRUE(tree.open("/"));
}

TEST_F(VfsTreeTest, test_open_nonexistent) {
    ASSERT_FALSE(tree.open("/nonexistent"));
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

/**************************************************************************
 * VfsTree::close
 *************************************************************************/
TEST_F(VfsTreeTest, test_close_opened) {
    auto fd = tree.open("/");
    ASSERT_TRUE(tree.close(fd.value));
}

TEST_F(VfsTreeTest, test_close_closed) {
    auto fd = tree.open("/");
    tree.close(fd.value);
    ASSERT_FALSE(tree.close(fd.value));
}

TEST_F(VfsTreeTest, test_close_neveropened) {
    auto fd = 42;
    tree.close(fd);
    ASSERT_FALSE(tree.close(fd));
}

/**************************************************************************
 * VfsTree::copy
 *************************************************************************/
TEST_F(VfsTreeTest, test_copy_within_mountpoint_same_level) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy_entry("/HOME/photo.jpg", "/HOME/picture.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/picture.jpg"));
}

TEST_F(VfsTreeTest, test_copy_within_mountpoint_different_level) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy_entry("/HOME/photo.jpg", "/HOME/images/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/photo.jpg"));
}

TEST_F(VfsTreeTest, test_copy_within_mountpoint_different_level2) {
    // setup
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamMountPoint>("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy_entry("/HOME/photo.jpg", "/HOME/images/"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/photo.jpg"));
}

/**************************************************************************
 * VfsTree::move
 *************************************************************************/
TEST_F(VfsTreeTest, test_rename_attachment) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));

    // test
    ASSERT_TRUE(tree.move_entry("/home", "/mydocs"));
    ASSERT_FALSE(tree.exists("/home"));
    ASSERT_TRUE(tree.exists("/mydocs"));
}

TEST_F(VfsTreeTest, test_rename_root) {
    // test
    ASSERT_FALSE(tree.move_entry("/", "root"));
}

TEST_F(VfsTreeTest, test_move_attachment) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.attach(mkdir("images"), "/"));

    // test
    ASSERT_TRUE(tree.move_entry("/images", "/home/pictures"));
    ASSERT_FALSE(tree.exists("/images"));
    ASSERT_TRUE(tree.exists("/home/pictures"));
}

TEST_F(VfsTreeTest, test_move_attachment_to_dir) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.attach(mkdir("images"), "/"));

    // test
    ASSERT_TRUE(tree.move_entry("/images", "/home"));
    ASSERT_FALSE(tree.exists("/images"));
    ASSERT_TRUE(tree.exists("/home/images"));
}
