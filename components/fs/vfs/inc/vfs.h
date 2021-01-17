/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VFS_H__
#define __VFS_H__

#include "sys/types.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "dirent.h"

#include "vos.h"
#include "vlist.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedefs */

struct file;
struct dir;
struct mount_point;

struct f_ops {
    int     (* open)      (struct file *fp, const char *path, int flags, int mode);
    int     (* close)     (struct file *fp);
    ssize_t (* read)      (struct file *fp, char *buf, size_t nbyte);
    ssize_t (* write)     (struct file *fp, const char *buf, size_t nbyte);
    off_t   (* lseek)     (struct file *fp, off_t offset, int whence);
    int     (* ioctl)     (struct file *fp, int request, va_list valist);
    int     (* sync)      (struct file *fp);
    int     (* stat)      (struct mount_point *mp, const char *path, struct stat *st);
    int     (* unlink)    (struct mount_point *mp, const char *path);
    int     (* rename)    (struct mount_point *mp, const char *from, const char *to);
    int     (* mkdir)     (struct mount_point *mp, const char *path, int mode);
    int     (* rmdir)     (struct mount_point *mp, const char *path);
    int     (* opendir)   (struct mount_point *mp, struct dir *dp, const char *path);
    int     (* readdir)   (struct mount_point *mp, struct dir *dp, struct dirent *de);
    int     (* closedir)  (struct mount_point *mp, struct dir *dp);
    int     (* mount)     (struct mount_point *mp, va_list valist);
    int     (* unmount)   (struct mount_point *mp, va_list valist);
};

struct file_system {
    const char          * fs_name;
    const struct f_ops  * fs_fops;
    volatile uint32_t     fs_refs;
    vdlist_t              fs_node;
};

struct mount_point {
    struct file_system  * mp_fs;
    char                * mp_path;
    vdlist_t              mp_node;
    vmutex_t              mp_lock;
    volatile uint32_t     mp_refs;
    uintptr_t             mp_data;
};

struct file {
    const struct f_ops  * fl_fops;
    int                   fl_flags;
    struct mount_point  * fl_mp;
    volatile uint32_t     fl_refs;
    uintptr_t             fl_data;
    char                * fl_path;
    off_t                 fl_offset;
};

/* file description */
struct fildes {
    struct file         * fd_file;
    int                   fd_flags;
    bool                  fd_used;
};

struct dir {
    struct mount_point  * dr_mp;
    struct dirent         dr_dent;
    uintptr_t             dr_data;
};

/* externs */

/* file */
extern int                vfs_open          (const char *path, int oflag, ...);
extern int                vfs_close         (int fd);
extern ssize_t            vfs_read          (int fd, void *buf, size_t nbyte);
extern ssize_t            vfs_pread         (int fd, void *buf, size_t nbyte, off_t offset);
extern ssize_t            vfs_write         (int fd, const void *buf, size_t nbyte);
extern ssize_t            vfs_pwrite        (int fd, const void *buf, size_t nbyte, off_t offset);
extern off_t              vfs_lseek         (int fd, off_t offset, int whence);
extern int                vfs_ioctl         (int fd, int request, ... /* arg */);
extern int                vfs_sync          (int fd);
extern int                vfs_stat          (const char *path, struct stat *st);
extern int                vfs_unlink        (const char *path);
/* file, dir */
extern int                vfs_rename        (const char *from, const char *to);
/* dir */
extern struct dir       * vfs_opendir       (const char *path);
extern struct dirent    * vfs_readdir       (struct dir *dp);
extern int                vfs_readdir_r     (struct dir *dp, struct dirent *entry, struct dirent **result);
extern int                vfs_closedir      (struct dir *dp);
extern int                vfs_mkdir         (const char *path, int mode);
extern int                vfs_rmdir         (const char *path);
/* other */
extern int                vfs_mount         (const char *fs_name, const char *mp_path, ...);
extern int                vfs_unmount       (const char *path, ...);
extern int                vfs_register      (struct file_system *fs);
extern int                vfs_unregister    (struct file_system *fs);
/* lib */
extern int                vfs_init          (void);
extern int                vfs_exit          (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __VFS_H__ */
