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

#include "vfs_yaffs.h"
#include "vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yportenv.h"


static int yafd_from_file(struct file *file)
{
    return (int)(uintptr_t)file->f_data;
}

static int yaffs2_op_open(struct file *file, const char *path, int flags, int mode)
{
    int fd;

    fd = yaffs_open(path, flags, mode);
    if (fd < 0) {
        vlog_error("yaffs_open() failed, ret=%d", fd);
        return -1;
    }
    file->f_data = (void *)(uintptr_t)fd;
    return 0;
}

static int yaffs2_op_close(struct file *file)
{
    int fd, ret;

    fd = yafd_from_file(file);

    ret = yaffs_close(fd);
    if (ret == 0) {
        file->f_data = NULL;
    }

    return ret;
}

static ssize_t yaffs2_op_read(struct file *file, char *buff, size_t bytes)
{
    int fd;

    fd = yafd_from_file(file);

    return yaffs_read(fd, buff, bytes);
}

static ssize_t yaffs2_op_write(struct file *file, const char *buff, size_t bytes)
{
    int fd;

    fd = yafd_from_file(file);

    return yaffs_write(fd, buff, bytes);
}

static off_t yaffs2_op_lseek(struct file *file, off_t off, int whence)
{
    int fd;

    fd = yafd_from_file(file);

    return yaffs_lseek(fd, off, whence);
}

static int yaffs2_op_stat(struct mnt *mp, const char *path, struct stat *stat)
{
    struct yaffs_stat s;
    int ret;

    ret = yaffs_stat(path, &s);
    if (ret < 0) {
        vlog_error("yaffs_stat() failed, ret=%d", ret);
        return -1;
    }
    stat->st_dev = s.st_dev;
    stat->st_ino = s.st_ino;
    stat->st_mode = s.st_mode;
    stat->st_nlink = s.st_nlink;
    stat->st_uid = s.st_uid;
    stat->st_gid = s.st_gid;
    stat->st_rdev = s.st_rdev;
    stat->st_size = s.st_size;
    stat->st_atime = s.yst_atime;
    stat->st_mtime = s.yst_mtime;
    stat->st_ctime = s.yst_ctime;
    stat->st_blksize = s.st_blksize;
    stat->st_blocks = s.st_blocks;

    return 0;
}

static int yaffs2_op_unlink(struct mnt *mp, const char *path)
{
    return yaffs_unlink(path);
}

static int yaffs2_op_rename(struct mnt *mp, const char *path_old, const char *path_new)
{
    return yaffs_rename(path_old, path_new);
}

static int yaffs2_op_sync(struct file *file)
{
    int fd;

    fd = yafd_from_file(file);

    return yaffs_flush(fd);
}

static int yaffs2_op_opendir(struct dir *dir, const char *path)
{
    yaffs_DIR *y_dir;

    y_dir = yaffs_opendir(path);
    if (y_dir == NULL) return -1;

    dir->d_data = (void *)y_dir;

    return 0;
}

static int yaffs2_op_readdir(struct dir *dir, struct dirent *dent)
{
    int     len;
    struct yaffs_dirent *y_dent;
    yaffs_DIR *y_dir = (yaffs_DIR *)dir->d_data;

    if (y_dir == NULL || dent == NULL) {
        return -1;
    }

    y_dent = yaffs_readdir(y_dir);
    if (y_dent == NULL) {
        return -1;
    }
    len = min(strlen(y_dent->d_name) + 1, sizeof(dent->d_name)) - 1;
    strncpy((char *)dent->d_name, (const char *)y_dent->d_name, len);
    dent->d_name[len] = '\0';
    dent->d_size = y_dent->d_reclen;
    dent->d_type = y_dent->d_type;

    return 0;
}

static int yaffs2_op_closedir(struct dir *dir)
{
    int ret;
    yaffs_DIR *y_dir = (yaffs_DIR *)dir->d_data;

    if (y_dir == NULL) return -1;

    ret = yaffs_closedir(y_dir);

    if (ret == 0) {
        dir->d_data = NULL;
    }

    return ret;
}

static int yaffs2_op_mkdir(struct mnt *mp, const char *path, int mode)
{
    return yaffs_mkdir(path, mode);
}

static int yaffs2_op_rmdir(struct mnt *mp, const char *path)
{
    return yaffs_rmdir(path);
}


static struct file_ops yaffs2_ops =
{
    yaffs2_op_open,
    yaffs2_op_close,
    yaffs2_op_read,
    yaffs2_op_write,
    yaffs2_op_lseek,
    yaffs2_op_stat,
    yaffs2_op_unlink,
    yaffs2_op_rename,
    NULL,               /* ioctl not supported for now */
    yaffs2_op_sync,
    yaffs2_op_opendir,
    yaffs2_op_readdir,
    yaffs2_op_closedir,
    yaffs2_op_mkdir,
    yaffs2_op_rmdir
};

static struct file_system yaffs2_fs =
{
    "yaffs2",
    &yaffs2_ops,
    NULL,
    0
};

int yaffs2_init(void)
{
    static int yaffs2_inited = 0;

    if (yaffs2_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&yaffs2_fs) != 0) {
        vlog_error("failed to register yaffs2!");
        return -1;
    }

    yaffs2_inited = 1;

    vlog_info("register yaffs2 done!");

    return 0;
}

int yaffs2_mount(const char *path)
{
    int ret;

    ret = yaffs_mount("");
    if (ret < 0) {
        vlog_error("yaffs_mount() failed, ret=%d", ret);
        return -1;
    }

    ret = vfs_mount("yaffs2", path, NULL);

    if (ret != 0)  {
        vlog_error("vfs_mount() failed, path=%s", path);
        yaffs_unmount("");
        return -1;
    }

    return 0;
}

int yaffs2_unmount(const char *path)
{
    int ret = yaffs_unmount("");
    if (ret < 0) {
        vlog_error("yaffs_unmount() failed");
    }
    ret = vfs_unmount(path);
    if (ret < 0) {
        vlog_error("vfs_unmount() failed, path=%s", path);
    }
    return ret;
}

