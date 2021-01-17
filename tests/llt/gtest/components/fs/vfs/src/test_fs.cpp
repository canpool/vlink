/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testfs.h"
#include "vconfig.h"

class TestFs : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static int __op_open(struct file *fp, const char *path, int flags, int mode)
{
    return -1;
}

static int __op_close(struct file *fp)
{
    return -1;
}

static ssize_t __op_read(struct file *fp, char *buf, size_t nbyte)
{
    return -1;
}

static ssize_t __op_write(struct file *fp, const char *buf, size_t nbyte)
{
    return -1;
}

static off_t __op_lseek(struct file *fp, off_t offset, int whence)
{
    return -1;
}

static int __op_sync(struct file *fp)
{
    return -1;
}

static int __op_stat(struct mount_point *mp, const char *path, struct stat *st)
{
    return -1;
}

static int __op_unlink(struct mount_point *mp, const char *path)
{
    return -1;
}

static int __op_rename(struct mount_point *mp, const char *from, const char *to)
{
    return -1;
}

static int __op_mkdir(struct mount_point *mp, const char *path, int mode)
{
    return -1;
}

static int __op_rmdir(struct mount_point *mp, const char *path)
{
    return -1;
}

static int __op_opendir(struct mount_point *mp, struct dir *dp, const char *path)
{
    return -1;
}

static int __op_readdir(struct mount_point *mp, struct dir *dp, struct dirent *de)
{
    return -1;
}

static int __op_closedir(struct mount_point *mp, struct dir *dp)
{
    return -1;
}

static int __op_mount(struct mount_point *mp, va_list valist)
{
    return 0;
}

static int __op_unmount(struct mount_point *mp, va_list valist)
{
    return 0;
}

static struct f_ops __fs_ops =
{
    __op_open,
    __op_close,
    __op_read,
    __op_write,
    __op_lseek,
    NULL,                   /* ioctl */
    __op_sync,
    __op_stat,
    __op_unlink,
    __op_rename,
    __op_mkdir,
    __op_rmdir,
    __op_opendir,
    __op_readdir,
    __op_closedir,
    __op_mount,
    __op_unmount
};

static struct file_system __fs =
{
    "testfs",
    &__fs_ops,
};

TEST_F(TestFs, register)
{
    __fs.fs_name = NULL;
    EXPECT_EQ(-1, vfs_register(&__fs));
    __fs.fs_name = "";
    EXPECT_EQ(-1, vfs_register(&__fs));
    __fs.fs_name = "#";
    EXPECT_EQ(-1, vfs_register(&__fs));
    __fs.fs_name = "0";
    EXPECT_EQ(0, vfs_register(&__fs));
    EXPECT_EQ(0, vfs_unregister(&__fs));
    __fs.fs_name = "A";
    EXPECT_EQ(0, vfs_register(&__fs));
    EXPECT_EQ(0, vfs_unregister(&__fs));
    __fs.fs_name = "a";
    EXPECT_EQ(0, vfs_register(&__fs));
    EXPECT_EQ(-1, vfs_register(&__fs));
    EXPECT_EQ(0, vfs_unregister(&__fs));
}

TEST_F(TestFs, mount)
{
    __fs.fs_name = "test";
    EXPECT_EQ(0, vfs_register(&__fs));

    EXPECT_EQ(-1, vfs_mount(NULL, NULL));
    EXPECT_EQ(-1, vfs_mount("t", "/"));
    EXPECT_EQ(-1, vfs_mount("t", "/fs"));

    EXPECT_EQ(0, vfs_mount("test", "/"));
    EXPECT_EQ(0, vfs_unmount("/"));

    EXPECT_EQ(0, vfs_mount("test", "/fs"));
    EXPECT_EQ(-1, vfs_mount("test", "/fs"));
    EXPECT_EQ(0, vfs_unmount("/fs"));

    EXPECT_EQ(0, vfs_mount("test", "//fs"));
    EXPECT_EQ(0, vfs_unmount("//fs"));

    EXPECT_EQ(0, vfs_mount("test", "//fs"));
    EXPECT_EQ(0, vfs_unmount("/fs"));

    EXPECT_EQ(0, vfs_mount("test", "/fs//"));
    EXPECT_EQ(0, vfs_unmount("/fs"));

    EXPECT_EQ(0, vfs_mount("test", "/fs//"));
    EXPECT_EQ(0, vfs_unmount("/fs/"));

    EXPECT_EQ(0, vfs_unregister(&__fs));
}
