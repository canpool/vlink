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

#include "vfs_mockfs.h"
#include "vfs.h"

#include <fcntl.h>      // open
#include <unistd.h>     // close, read, write, lseek, fsync
#include <sys/ioctl.h>


int mockfs_op_open(struct file *fp, const char *path, int flags, int mode)
{
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return -1;
    } else {
        fp->f_data = (void *)fd;
    }
    return 0;
}

int mockfs_op_close(struct file *fp)
{
    return close((int)fp->f_data);
}

ssize_t mockfs_op_read(struct file *fp, char *buf, size_t len)
{
    return read((int)fp->f_data, buf, len);
}

ssize_t mockfs_op_write(struct file *fp, const char *buf, size_t len)
{
    return write((int)fp->f_data, buf, len);
}

off_t mockfs_op_lseek(struct file *fp, off_t off, int whence)
{
    return lseek((int)fp->f_data, off, whence);
}

int mockfs_op_stat(struct mnt *mp, const char *path, struct stat *st)
{
    return stat(path, st);
}

int mockfs_op_unlink(struct mnt *mp, const char *path)
{
    return unlink(path);
}

int mockfs_op_rename(struct mnt *mp, const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

int mockfs_op_sync(struct file *fp)
{
    return fsync((int)fp->f_data);
}

int mockfs_op_opendir(struct dir *dp, const char *path)
{
    DIR *d = opendir(path);
    if (d == NULL) {
        return -1;
    }
    dp->d_data = d;
    return 0;
}

int mockfs_op_readdir(struct dir *dp, struct dirent *de)
{
    DIR *d = (DIR *)dp->d_data;
    struct dirent *d_temp = NULL;

    d_temp = readdir(d);
    if (d_temp == NULL) {
        return -1;
    }
    *de = *d_temp;
    return 0;
}

int mockfs_op_closedir(struct dir *dp)
{
    DIR *d = (DIR *)dp->d_data;

    return closedir(d);
}

int mockfs_op_mkdir(struct mnt *mp, const char *path, int mode)
{
    return mkdir(path, mode);
}

int mockfs_op_rmdir(struct mnt *mp, const char *path)
{
    return rmdir(path);
}

static struct file_ops mockfs_ops =
{
    mockfs_op_open,
    mockfs_op_close,
    mockfs_op_read,
    mockfs_op_write,
    mockfs_op_lseek,
    mockfs_op_stat,
    mockfs_op_unlink,
    mockfs_op_rename,
    NULL,               /* ioctl not supported for now */
    mockfs_op_sync,
    mockfs_op_opendir,
    mockfs_op_readdir,
    mockfs_op_closedir,
    mockfs_op_mkdir,
    mockfs_op_rmdir
};

static struct file_system mockfs_fs =
{
    "mockfs",
    &mockfs_ops,
    NULL,
    0
};


int mockfs_init(void)
{
    static int mockfs_inited = 0;

    if (mockfs_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&mockfs_fs) != 0) {
        printf("failed to register mockfs!\n");
        return -1;
    }

    mockfs_inited = 1;

    printf("register mockfs done!\n");

    return 0;
}

int mockfs_mount(const char *path, void *data)
{
    int ret = vfs_mount("mockfs", path, data);
    return ret;
}

int mockfs_unmount(const char *path)
{
    int ret = vfs_unmount(path);

    return ret;
}

