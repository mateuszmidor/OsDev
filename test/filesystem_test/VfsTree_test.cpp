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
#include "VfsRamDummyFileEntry.h"
#include "VfsRamMountPoint.h"
#include "KernelLog.h"

using namespace filesystem;

class VfsTreeTest : public ::testing::Test {
protected:
    VfsTree tree;

    VfsRamDirectoryEntryPtr mkdir(const char name[]) {
        return std::make_shared<VfsRamDirectoryEntry>(name);
    }

    std::shared_ptr<VfsRamDummyFileEntry> mkfile(const char name[]) {
        return std::make_shared<VfsRamDummyFileEntry>(name);
    }

    std::shared_ptr<VfsRamMountPoint> mkmountpoint(const char name[]) {
        return std::make_shared<VfsRamMountPoint>(name);
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
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    // test
    ASSERT_TRUE(tree.attach(mkdir("images"), "/HOME"));
    ASSERT_TRUE(tree.exists("/HOME/images"));
}

TEST_F(VfsTreeTest, test_attach_in_multilevel_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));

    ASSERT_TRUE(tree.create("/HOME/images/cars", true));
    ASSERT_TRUE(tree.exists("/HOME/images/cars"));

    ASSERT_TRUE(tree.attach(mkmountpoint("VOLVO"), "/HOME/images/cars"));
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
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    // test
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));
}

TEST_F(VfsTreeTest, test_create_in_multilevel_mountpoint) {
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.exists("/HOME"));

    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));

    ASSERT_TRUE(tree.create("/HOME/images/cars", true));
    ASSERT_TRUE(tree.exists("/HOME/images/cars"));

    ASSERT_TRUE(tree.attach(mkmountpoint("VOLVO"), "/HOME/images/cars"));
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
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.exists("/HOME/images"));

    // test
    ASSERT_TRUE(tree.remove("/HOME/images"));
    ASSERT_FALSE(tree.exists("/HOME/images"));
}

/**************************************************************************
 * VfsTree::copy
 *************************************************************************/
TEST_F(VfsTreeTest, test_copy_within_mountpoint_same_level) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy("/HOME/photo.jpg", "/HOME/picture.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/picture.jpg"));
}

TEST_F(VfsTreeTest, test_copy_within_mountpoint_different_level_to_file) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy("/HOME/photo.jpg", "/HOME/images/picture.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/picture.jpg"));
}

TEST_F(VfsTreeTest, test_copy_within_mountpoint_different_level_to_dir) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy("/HOME/photo.jpg", "/HOME/images/"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/photo.jpg"));
}

TEST_F(VfsTreeTest, test_copy_within_different_mountpoints) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.attach(mkmountpoint("IMAGES"), "/"));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.copy("/HOME/photo.jpg", "/IMAGES/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/IMAGES/photo.jpg"));
}

TEST_F(VfsTreeTest, test_copy_attachment_to_mountpoint) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.attach(std::make_shared<VfsRamDummyFileEntry>("photo.jpg"), "/"));

    // test
    ASSERT_TRUE(tree.copy("/photo.jpg", "/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/photo.jpg"));
}

// destination must point to a mountpoint so can't just copy attachment to another attachment
TEST_F(VfsTreeTest, test_copy_attachment_to_attachment_cantdo) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));

    // test
    ASSERT_FALSE(tree.copy("/photo.jpg", "/picture.jpg"));
    ASSERT_TRUE(tree.exists("/photo.jpg"));
    ASSERT_FALSE(tree.exists("/picture.jpg"));
}

/**************************************************************************
 * VfsTree::move
 *************************************************************************/
TEST_F(VfsTreeTest, test_rename_attachment) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));

    // test
    ASSERT_TRUE(tree.move("/home", "/mydocs"));
    ASSERT_FALSE(tree.exists("/home"));
    ASSERT_TRUE(tree.exists("/mydocs"));
}

TEST_F(VfsTreeTest, test_rename_root) {
    // test
    ASSERT_FALSE(tree.move("/", "root"));
}

TEST_F(VfsTreeTest, test_move_attachment_to_file) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.attach(mkfile("image.jpg"), "/"));

    // test
    ASSERT_TRUE(tree.move("/image.jpg", "/home/picture.jpg"));
    ASSERT_FALSE(tree.exists("/image.jpg"));
    ASSERT_TRUE(tree.exists("/home/picture.jpg"));
}

TEST_F(VfsTreeTest, test_move_attachment_to_dir) {
    // setup
    ASSERT_TRUE(tree.attach(mkdir("home"), "/"));
    ASSERT_TRUE(tree.attach(mkfile("image.jpg"), "/"));

    // test
    ASSERT_TRUE(tree.move("/image.jpg", "/home"));
    ASSERT_FALSE(tree.exists("/image.jpg"));
    ASSERT_TRUE(tree.exists("/home/image.jpg"));
}

TEST_F(VfsTreeTest, test_move_within_mountpoint_same_level) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.move("/HOME/photo.jpg", "/HOME/picture.jpg"));
    ASSERT_FALSE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/picture.jpg"));
}

TEST_F(VfsTreeTest, test_move_within_mountpoint_different_level) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.move("/HOME/photo.jpg", "/HOME/images/photo.jpg"));
    ASSERT_FALSE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/photo.jpg"));
}

TEST_F(VfsTreeTest, test_move_within_mountpoint_different_level_to_dir) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.create("/HOME/images", true));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.move("/HOME/photo.jpg", "/HOME/images/"));
    ASSERT_FALSE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/HOME/images/photo.jpg"));
}

TEST_F(VfsTreeTest, test_move_within_different_mountpoints) {
    // setup
    ASSERT_TRUE(tree.attach(mkmountpoint("HOME"), "/"));
    ASSERT_TRUE(tree.attach(mkmountpoint("IMAGES"), "/"));
    ASSERT_TRUE(tree.create("/HOME/photo.jpg", false));

    // test
    ASSERT_TRUE(tree.move("/HOME/photo.jpg", "/IMAGES/photo.jpg"));
    ASSERT_FALSE(tree.exists("/HOME/photo.jpg"));
    ASSERT_TRUE(tree.exists("/IMAGES/photo.jpg"));
}

/**************************************************************************
 * VfsTree::get_cached
 *************************************************************************/
TEST_F(VfsTreeTest, test_get_cached_basic) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));

    // test
    auto cached = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached);
    ASSERT_TRUE(cached->get_name() == "photo.jpg");
}

TEST_F(VfsTreeTest, test_get_cached_twice_brings_same_entry) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));

    // test
    auto cached1 = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached1);

    auto cached2 = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached2);

    ASSERT_TRUE(cached1 == cached2);
}

/**************************************************************************
 * VfsTree::release_cached
 *************************************************************************/
TEST_F(VfsTreeTest, test_release_cached_basic) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));
    auto cached = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached);

    // test
    ASSERT_TRUE(tree.release_cached(cached));
}

TEST_F(VfsTreeTest, test_release_cached_open_fails) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));
    auto cached = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached);
    cached->open_count = 1;

    // test
    ASSERT_FALSE(tree.release_cached(cached));
}

TEST_F(VfsTreeTest, test_release_cached_with_attachment_fails) {
    // setup
    ASSERT_TRUE(tree.attach(mkfile("photo.jpg"), "/"));
    auto cached = tree.get_cached("/photo.jpg");
    ASSERT_TRUE(cached);
    cached->attach_entry(mkfile("attachment"));

    // test
    ASSERT_FALSE(tree.release_cached(cached));
}
