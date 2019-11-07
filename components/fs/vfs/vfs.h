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

#ifndef __VFS_H__
#define __VFS_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "sys/types.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "dirent.h"

#include "vos.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct file;
struct stat;
struct mnt;
struct dir;
struct dirent;

struct file_ops
{
    int     (*open)     (struct file *fp, const char *path, int flags, int mode);
    int     (*close)    (struct file *fp);
    ssize_t (*read)     (struct file *fp, char *buf, size_t len);
    ssize_t (*write)    (struct file *fp, const char *buf, size_t len);
    off_t   (*lseek)    (struct file *fp, off_t off, int whence);
    int     (*stat)     (struct mnt  *mp, const char *path, struct stat *st);
    int     (*unlink)   (struct mnt  *mp, const char *path);
    int     (*rename)   (struct mnt  *mp, const char *oldpath, const char *newpath);
    int     (*ioctl)    (struct file *fp, int request, unsigned long arg);
    int     (*sync)     (struct file *fp);
    int     (*opendir)  (struct dir  *dp, const char *path);
    int     (*readdir)  (struct dir  *dp, struct dirent *de);
    int     (*closedir) (struct dir  *dp);
    int     (*mkdir)    (struct mnt  *mp, const char *path, int mode);
    int     (*rmdir)    (struct mnt  *mp, const char *path);
};

struct file_system
{
    const char          *fs_name;
    struct file_ops     *fs_fops;
    struct file_system  *fs_next;
    volatile uint32_t    fs_refs;
};

/* mount point */
struct mnt
{
    struct file_system  *mp_fs;
    struct mnt          *mp_next;
    char                *mp_path;
    volatile uint32_t   *mp_refs;
    vmutex_t             mp_lock;
    void                *mp_data;
};

struct file
{
    struct file_ops     *f_fops;
    int                  f_flags;
    int                  f_status;
    char                *f_path;
    struct mnt          *f_mp;
    off_t                f_offset;
    volatile uint32_t    f_refs;
    void                *f_data;
};

struct dir
{
    struct mnt          *d_mp;
    struct dirent        d_dent;
    void                *d_data;
};

/* file */
int               vfs_open      (const char *o_path, int flags, ...);
int               vfs_close     (int fd);
ssize_t           vfs_read      (int fd, void *buf, size_t bytes);
ssize_t           vfs_write     (int fd, const char *buf, size_t bytes);
off_t             vfs_lseek     (int fd, off_t off, int whence);
int               vfs_ioctl     (int fd, int request, ...);
int               vfs_sync      (int fd);
int               vfs_stat      (const char *s_path, struct stat *stat);
int               vfs_unlink    (const char *u_path);
/* file, dir */
int               vfs_rename    (const char *oldpath, const char *newpath);
/* dir */
struct dir      * vfs_opendir   (const char *o_path);
struct dirent   * vfs_readdir   (struct dir *dir);
int               vfs_closedir  (struct dir *dir);
int               vfs_mkdir     (const char *c_path, int mode);
int               vfs_rmdir     (const char *c_path);

/* other */
int               vfs_mount     (const char *fs_name, const char *mp_path, void *data);
int               vfs_unmount   (const char *path);

int               vfs_init      (void);

int               vfs_register  (struct file_system *fs);
int               vfs_unregister(struct file_system *fs);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __VFS_H__ */
