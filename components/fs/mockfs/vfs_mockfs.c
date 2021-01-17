/**
 * Copyright (c) [2020] China Canpool Team, All rights reserved.
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

#include "vfs_mockfs.h"
#include "vfs.h"
#include "vmodule.h"

#include <fcntl.h>      // open
#include <unistd.h>     // close, read, write, lseek, fsync
#include <sys/ioctl.h>

static int __op_open(struct file *fp, const char *path, int flags, int mode)
{
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return -1;
    }
    fp->fl_data = (uintptr_t)fd;
    return 0;
}

static int __op_close(struct file *fp)
{
    return close((int)fp->fl_data);
}

static ssize_t __op_read(struct file *fp, char *buf, size_t nbyte)
{
    return read((int)fp->fl_data, buf, nbyte);
}

static ssize_t __op_write(struct file *fp, const char *buf, size_t nbyte)
{
    return write((int)fp->fl_data, buf, nbyte);
}

static off_t __op_lseek(struct file *fp, off_t offset, int whence)
{
    return lseek((int)fp->fl_data, offset, whence);
}

static int __op_sync(struct file *fp)
{
    return fsync((int)fp->fl_data);
}

static int __op_stat(struct mount_point *mp, const char *path, struct stat *st)
{
    return stat(path, st);
}

static int __op_unlink(struct mount_point *mp, const char *path)
{
    return unlink(path);
}

static int __op_rename(struct mount_point *mp, const char *from, const char *to)
{
    return rename(from, to);
}

static int __op_mkdir(struct mount_point *mp, const char *path, int mode)
{
    return mkdir(path, mode);
}

static int __op_rmdir(struct mount_point *mp, const char *path)
{
    return rmdir(path);
}

static int __op_opendir(struct mount_point *mp, struct dir *dp, const char *path)
{
    DIR *d = opendir(path);
    if (d == NULL) {
        return -1;
    }
    dp->dr_data = (uintptr_t)d;

    return 0;
}

static int __op_readdir(struct mount_point *mp, struct dir *dp, struct dirent *de)
{
    DIR           * d       = (DIR *)dp->dr_data;
    struct dirent * de_temp = NULL;

    de_temp = readdir(d);
    if (de_temp == NULL) {
        return -1;
    }
    *de = *de_temp;

    return 0;
}

static int __op_closedir(struct mount_point *mp, struct dir *dp)
{
    DIR *d = (DIR *)dp->dr_data;

    return closedir(d);
}

static int __op_mount(struct mount_point *mp, va_list valist)
{
    return 0;
}

static int __op_unmount(struct mount_point *mp, va_list valist)
{
    return 0;
}

static struct f_ops mockfs_ops =
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

static struct file_system mockfs_fs =
{
    "mockfs",
    &mockfs_ops,
};

int mockfs_init(void)
{
    return vfs_register(&mockfs_fs);
}

int mockfs_exit(void)
{
    return vfs_unregister(&mockfs_fs);
}

VMODULE_DEF(VMODULE_POSTLIB, mockfs_init, mockfs_exit);

int mockfs_mount(const char *path)
{
    return vfs_mount("mockfs", path);
}

int mockfs_unmount(const char *path)
{
    return vfs_unmount(path);
}
