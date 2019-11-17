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

#include "vfs_uffs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uffs/uffs_fd.h"
#include "uffs/uffs_mtb.h"
#include "uffs/uffs_public.h"

#include "vfs.h"

static int ufd_from_file(struct file *file)
{
    return (int)(uintptr_t)file->f_data;
}

static int uffs_flag_conv(int flags)
{
    int acc_mode, res = 0;
    acc_mode = flags & O_ACCMODE;
    if (acc_mode == O_RDONLY) {
        res |= UO_RDONLY;
    } else if (acc_mode == O_WRONLY) {
        res |= UO_WRONLY;
    } else if (acc_mode == O_RDWR) {
        res |= UO_RDWR;
    }

    if ((flags & O_CREAT) && (flags & O_EXCL)) {
        res |= UO_CREATE | UO_EXCL;
    } else if ((flags & O_CREAT) && (flags & O_TRUNC)) {
        res |= UO_CREATE | UO_TRUNC;
    } else if (flags & O_CREAT) {
        res |= UO_CREATE;
    } else if (flags & O_APPEND) {
        res |= UO_CREATE | UO_APPEND;
    }
    return res;
}

static int uffs_whence_conv(int whence)
{
    int res = USEEK_SET;

    if (whence == SEEK_SET) {
        res = USEEK_SET;
    } else if (whence == SEEK_CUR) {
        res = USEEK_CUR;
    } else if (whence == SEEK_END) {
        res = USEEK_END;
    }
    return res;
}

static int uffs_op_open(struct file *file, const char *path, int flags, int mode)
{
    int fd;

    fd = uffs_open(path, uffs_flag_conv(flags), mode);
    if (fd < 0) {
        vlog_error("uffs_open() failed, ret=%d", fd);
        return -1;
    }
    file->f_data = (void *)(uintptr_t)fd;
    return 0;
}

static int uffs_op_close(struct file *file)
{
    int fd, ret;

    fd = ufd_from_file(file);

    ret = uffs_close(fd);
    if (ret == 0) {
        file->f_data = NULL;
    }

    return ret;
}

static ssize_t uffs_op_read(struct file *file, char *buff, size_t bytes)
{
    int fd;

    fd = ufd_from_file(file);

    return (ssize_t)uffs_read(fd, buff, bytes);
}

static ssize_t uffs_op_write(struct file *file, const char *buff, size_t bytes)
{
    int fd;

    fd = ufd_from_file(file);

    return (ssize_t)uffs_write(fd, buff, bytes);
}

static off_t uffs_op_lseek(struct file *file, off_t off, int whence)
{
    int fd;

    fd = ufd_from_file(file);

    return (off_t)uffs_seek(fd, off, uffs_whence_conv(whence));
}

static int uffs_op_stat(struct mnt *mp, const char *path, struct stat *stat)
{
    struct uffs_stat s;
    int ret;

    ret = uffs_stat(path, &s);
    if (ret < 0) {
        vlog_error("uffs_stat() failed, ret=%d", ret);
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
    //    stat->st_atime = s.st_atime;
    //    stat->st_mtime = s.st_mtime;
    //    stat->st_ctime = s.st_ctime;
    stat->st_blksize = s.st_blksize;
    stat->st_blocks = s.st_blocks;

    return 0;
}

static int uffs_op_unlink(struct mnt *mp, const char *path)
{
    return uffs_remove(path);
}

static int uffs_op_rename(struct mnt *mp, const char *path_old, const char *path_new)
{
    return uffs_rename(path_old, path_new);
}

static int uffs_op_sync(struct file *file)
{
    int fd;

    fd = ufd_from_file(file);

    return uffs_flush(fd);
}

static int uffs_op_opendir(struct dir *dir, const char *path)
{
    uffs_DIR *u_dir;

    u_dir = uffs_opendir(path);
    if (u_dir == NULL)
        return -1;

    dir->d_data = (void *)u_dir;

    return 0;
}

static int uffs_op_readdir(struct dir *dir, struct dirent *dent)
{
    int len;
    struct uffs_dirent *u_dent;
    uffs_DIR *u_dir = (uffs_DIR *)dir->d_data;

    if (u_dir == NULL || dent == NULL) {
        return -1;
    }

    u_dent = uffs_readdir(u_dir);
    if (u_dent == NULL) {
        return -1;
    }
    len = min(strlen(u_dent->d_name) + 1, sizeof(dent->d_name)) - 1;
    strncpy((char *)dent->d_name, (const char *)u_dent->d_name, len);
    dent->d_name[len] = '\0';
    dent->d_size = u_dent->d_reclen;
    if (u_dent->d_type & FILE_ATTR_DIR) {
        dent->d_type = DT_DIR;
    } else {
        dent->d_type = DT_REG;
    }

    return 0;
}

static int uffs_op_closedir(struct dir *dir)
{
    int ret;
    uffs_DIR *u_dir = (uffs_DIR *)dir->d_data;

    if (u_dir == NULL)
        return -1;

    ret = uffs_closedir(u_dir);

    if (ret == 0) {
        dir->d_data = NULL;
    }

    return ret;
}

static int uffs_op_mkdir(struct mnt *mp, const char *path, int mode)
{
    return uffs_mkdir(path, mode);
}

static int uffs_op_rmdir(struct mnt *mp, const char *path)
{
    return uffs_rmdir(path);
}

static struct file_ops uffs_ops = {
    uffs_op_open,
    uffs_op_close,
    uffs_op_read,
    uffs_op_write,
    uffs_op_lseek,
    uffs_op_stat,
    uffs_op_unlink,
    uffs_op_rename,
    NULL, /* ioctl not supported for now */
    uffs_op_sync,
    uffs_op_opendir,
    uffs_op_readdir,
    uffs_op_closedir,
    uffs_op_mkdir,
    uffs_op_rmdir
};

static struct file_system uffs_fs = {
    "uffs",
    &uffs_ops,
    NULL,
    0
};

int uffs_init(void)
{
    static int uffs_inited = 0;

    if (uffs_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&uffs_fs) != 0) {
        vlog_error("failed to register uffs!");
        return -1;
    }

    uffs_inited = 1;

    vlog_info("register uffs done!");

    return 0;
}

int uffs_mount(const char *path)
{
    int ret;

    ret = uffs_Mount("");
    if (ret < 0) {
        vlog_error("uffs_Mount() failed, ret=%d", ret);
        return -1;
    }

    ret = vfs_mount("uffs", path, NULL);

    if (ret != 0) {
        vlog_error("vfs_mount() failed, path=%s", path);
        uffs_UnMount("");
        return -1;
    }

    return 0;
}

int uffs_unmount(const char *path)
{
    int ret = uffs_UnMount("");
    if (ret < 0) {
        vlog_error("uffs_UnMount() failed, ret=%d", ret);
    }
    ret = vfs_unmount(path);
    if (ret < 0) {
        vlog_error("vfs_unmount() failed, path=%s", path);
    }
    return ret;
}
