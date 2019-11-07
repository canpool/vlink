/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "testfs.h"

#include "gtest/gtest.h"

#include "vfs.h"
#include "vfs_mockfs.h"


class TestVfs : public ::testing::Test
{
protected:
    void SetUp() {
        mockfs_init();
        mockfs_mount("/mockfs/", NULL);
	};

	void TearDown() {
	    mockfs_unmount("/mockfs/");
	};
};

TEST_F(TestVfs, OpenFile)
{
    ASSERT_EQ(-1, vfs_open(NULL, O_RDONLY));
    ASSERT_EQ(-1, vfs_open("", O_RDONLY));
    ASSERT_EQ(-1, vfs_open("/", O_RDONLY));
    ASSERT_EQ(-1, vfs_open("/mockfs", O_RDONLY));
    ASSERT_EQ(-1, vfs_open("/mockfs/d/", O_RDONLY));

    const char *f_name = "/mockfs/f_open.txt";

    int fd = vfs_open(f_name, O_RDWR | O_CREAT);
    ASSERT_GE(fd, 0);

    int ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    ret = vfs_unlink(f_name);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, CloseFile)
{
    ASSERT_EQ(-1, vfs_close(-1));
    ASSERT_EQ(-1, vfs_close(0));
    ASSERT_EQ(-1, vfs_close(256));

    const char *f_name = "/mockfs/f_close.txt";

    int fd = vfs_open(f_name, O_RDWR | O_CREAT);
    ASSERT_GE(fd, 0);

    int ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    ret = vfs_unlink(f_name);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, FileOperation)
{
    const char *f_name = "/mockfs/f_opt.txt";
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // open
    int fd = vfs_open(f_name, O_RDWR | O_CREAT | O_TRUNC, mode);
    ASSERT_GE(fd, 0);

    // write
    const char *hello = "hello file\n";
    ssize_t size = vfs_write(fd, hello, strlen(hello));
    ASSERT_EQ(strlen(hello), size);
    printf("write : %s", hello);

    // read null from current position
    char buf[20] = {0};
    size = vfs_read(fd, buf, sizeof(buf));
    ASSERT_EQ(0, size);

    // set position to head
    off_t offset = vfs_lseek(fd, 0, SEEK_SET);
    ASSERT_EQ(0, offset);

    // read again
    size = vfs_read(fd, buf, sizeof(buf));
    ASSERT_EQ(strlen(hello), size);
    printf("read  : %s", buf);

    // close
    int ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    ret = vfs_unlink(f_name);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, OpenDir)
{
    int ret = 0;
    struct dir *d = NULL;
    const char *d_name = "/mockfs/d_open";

    d = vfs_opendir("/mockfs/n");
    ASSERT_TRUE(d == NULL);

    ret = vfs_mkdir(d_name, S_IRWXU | S_IROTH | S_IXOTH);
    ASSERT_EQ(0, ret);

    d = vfs_opendir(d_name);
    ASSERT_TRUE(d != NULL);

    ret = vfs_closedir(d);
    ASSERT_EQ(0, ret);

    // Applications should use rmdir() to remove a directory
    ret = vfs_unlink(d_name);
    ASSERT_EQ(-1, ret);

    ret = vfs_rmdir(d_name);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, PrintDir)
{
/*
    .
    |--- .2/
    |    |-- 2.txt
    |--- 1/
    |    |-- 1.txt
    |--- 3/
    |    |-- 3.txt
    |    |-- 3_1/
    |        |-- 3_1.txt
*/
    system("mkdir -p d/1");
    system("mkdir -p d/.2");
    system("mkdir -p d/3/3_1");
    system("touch d/1/1.txt");
    system("touch d/.2/2.txt");
    system("touch d/3/3.txt");
    system("touch d/3/3_1/3_1.txt");

    const char *d_name = "/mockfs/d";

    print_dir(d_name, 1);

    // The directory shall be removed only if it is an empty directory
    int ret = vfs_rmdir(d_name);
    ASSERT_EQ(-1, ret);

    system("rm -rf d");
}

TEST_F(TestVfs, RenameFile)
{
    int ret = -1;
    const char *f_oldname = "/mockfs/f_old.txt";
    const char *f_newname = "/mockfs/f_new.txt";

    int fd = vfs_open(f_oldname, O_RDWR | O_CREAT);
    ASSERT_GE(fd, 0);

    // file is opened
    ret = vfs_rename(f_oldname, f_newname);
    ASSERT_EQ(-1, ret);

    ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    ret = vfs_rename(f_oldname, f_newname);
    ASSERT_EQ(0, ret);

    // old file is not exist
    ret = vfs_unlink(f_oldname);
    ASSERT_EQ(-1, ret);

    ret = vfs_unlink(f_newname);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, RenameDir)
{
    int ret = -1;
    int fd = -1;
    struct dir *d = NULL;
    const char *d_oldname = "/mockfs/d_old";
    const char *d_newname = "/mockfs/d_new";
    const char *f_oldname = "/mockfs/d_old/f.txt";
    const char *f_newname = "/mockfs/d_new/f.txt";

    system("rm -rf d_old d_new");

    // d_old is not exist
    fd = vfs_open(f_oldname, O_RDWR | O_CREAT);
    ASSERT_EQ(-1, fd);

    // mkdir d_old
    ret = vfs_mkdir(d_oldname, S_IRWXU | S_IROTH | S_IXOTH);
    ASSERT_EQ(0, ret);

    // open file
    fd = vfs_open(f_oldname, O_RDWR | O_CREAT);
    ASSERT_GE(fd, 0);

    // file is opened, can't rename dir
    ret = vfs_rename(d_oldname, d_newname);
    ASSERT_EQ(-1, ret);

    ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    d = vfs_opendir(d_oldname);
    ASSERT_TRUE(d != NULL);

    // dir is opened, can rename dir
    ret = vfs_rename(d_oldname, d_newname);
    ASSERT_EQ(0, ret);

    ret = vfs_closedir(d);
    ASSERT_EQ(0, ret);

    // old dir is not exist
    ret = vfs_rmdir(d_oldname);
    ASSERT_EQ(-1, ret);

    // new dir is not empty
    ret = vfs_rmdir(d_newname);
    ASSERT_EQ(-1, ret);

    // remove file
    ret = vfs_unlink(f_newname);
    ASSERT_EQ(0, ret);

    // remove new dir
    ret = vfs_rmdir(d_newname);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, FileStat)
{
    int ret = -1;
    struct stat st;
    const char *f_name = "/mockfs/f_stat.txt";
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    memset(&st, 0, sizeof(struct stat));

    // open
    int fd = vfs_open(f_name, O_RDWR | O_CREAT | O_TRUNC, mode);
    ASSERT_GE(fd, 0);

    // write
    const char *hello = "hello file\n";
    ssize_t size = vfs_write(fd, hello, strlen(hello));
    ASSERT_EQ(strlen(hello), size);

    // close
    ret = vfs_close(fd);
    ASSERT_EQ(0, ret);

    ret = vfs_stat(f_name, &st);
    ASSERT_EQ(0, ret);

//  -rwxrwxrwx   1 root     root            11 Fri Sep 13 11:06:12 2019 f_stat.txt
    print_stat("f_stat.txt", &st);

    ret = vfs_unlink(f_name);
    ASSERT_EQ(0, ret);
}

TEST_F(TestVfs, DirStat)
{
/*
    drwxrwxrwx   1 root     root             0 Fri Sep 13 11:06:12 2019 d/.2
    -rwxrwxrwx   1 root     root            50 Fri Sep 13 11:06:12 2019 d/.2/2.txt
    drwxrwxrwx   1 root     root             0 Fri Sep 13 11:06:12 2019 d/1
    -rwxrwxrwx   1 root     root            51 Fri Sep 13 11:06:12 2019 d/1/1.txt
    drwxrwxrwx   1 root     root             0 Fri Sep 13 11:06:12 2019 d/3
    -rwxrwxrwx   1 root     root            51 Fri Sep 13 11:06:12 2019 d/3/3.txt
    drwxrwxrwx   1 root     root             0 Fri Sep 13 11:06:12 2019 d/3/3_1
    -rwxrwxrwx   1 root     root            45 Fri Sep 13 11:06:12 2019 d/3/3_1/3_1.txt
*/
    system("mkdir -p d/1");
    system("mkdir -p d/.2");
    system("mkdir -p d/3/3_1");
    system("touch d/1/1.txt");
    system("touch d/.2/2.txt");
    system("touch d/3/3.txt");
    system("touch d/3/3_1/3_1.txt");

    system("echo 11111111111111111111111111111111111111111111111111 > d/1/1.txt");
    system("echo 2222222222222222222222222222222222222222222222222 > d/.2/2.txt");
    system("echo 33333333333333333333333333333333333333333333333333 > d/3/3.txt");
    system("echo 31313131313131313131313131313131313131313131 > d/3/3_1/3_1.txt");

    const char *d_name = "/mockfs/d";

    list_stat(d_name);

    system("rm -rf d");
}

