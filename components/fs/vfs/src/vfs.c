/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vfs.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "vbug.h"
#include "vconfig.h"
#include "vmodule.h"

#ifndef CONFIG_OPEN_MAX
#define CONFIG_OPEN_MAX             16
#endif

static struct fildes fd_table[CONFIG_OPEN_MAX] = {0};

static vdlist_t fs_head = VDLIST_INIT(fs_head);
static vdlist_t mp_head = VDLIST_INIT(mp_head);
static vmutex_t fs_lock;

static bool __fs_name_valid(const char *fs_name)
{
    int         ch;
    const char *name = fs_name;

    while ((ch = *name) != '\0') {
        if (!islower(ch | 0x20) && !isdigit(ch)) {
            return false;
        }
        name++;
    }

    return (name != fs_name);
}

static struct file_system *__get_fs_by_name(const char *name)
{
    vdlist_t           * itr;
    struct file_system * fs = NULL;

    vdlist_foreach(itr, &fs_head) {
        fs = container_of(itr, struct file_system, fs_node);
        if (strcmp(name, fs->fs_name) == 0) {
            return fs;
        }
    }

    return NULL;
}

/**
 * @brief get the new path that is formatted
 *
 * eg:
 * 1) "//fs/"   =>  "/fs"
 * 2) "/fs//"   =>  "/fs"
 */
static char *__get_new_path(const char *path)
{
    char * p    = (char *)vmem_malloc(strlen(path) + 1);
    char * t    = p;
    char   last = '\0';
    char   ch;

    if (p == NULL) {
        return NULL;
    }

    while ((ch = *path++) != '\0') {
        /* filter out duplicate '/' int path */
        if ((ch == '/') && (last == '/')) {
            continue;
        }

        last = ch;
        *t++ = ch;
    }

    /* remove the last '/' in path */
    if (last == '/') {
        t[-1] = '\0';
    } else {
        t[0] = '\0';
    }

    return p;
}

/**
 * @brief get mount_point struct by using path to match mp_path
 *
 * note:
 * 1) this routine must be invoked with the fs_lock locked
 * 2) there is no '//' in <path>, see __get_new_path()
 *
 * @param path      [IN]    the formatted path, like: "/fs", "/fs/file"
 * @param rela_path [OUT]   the path relative to mp_path
 *
 * @return the mount_point struct pointer or NULL
 *
 * remarks:
 * 1) if retval == NULL,      then the path is invalid
 * 2) if **rela_path == '\0', then the path is equal to mp_path
 */
static struct mount_point *__path_resolve(const char *path, const char **rela_path)
{
    vdlist_t           * itr;
    struct mount_point * best     = NULL;
    size_t               best_len = 0;

    vdlist_foreach(itr, &mp_head) {
        size_t               len;   /* mount point path string length */
        struct mount_point * mp;
        const char         * sub;

        mp  = container_of(itr, struct mount_point, mp_node);
        sub = strstr(path, mp->mp_path);
        if (sub == NULL || sub != path) {
            continue;
        }

        len = strlen(mp->mp_path);
        if (path[len] == '\0') {
            best_len = len - 1; /* so, best_len + 1 == len, see the end */
            best     = mp;
            break;
        }

        if ((len > best_len) && (path[len] == '/')) {
            best_len = len;
            best     = mp;
        }
    }

    if (rela_path != NULL) {
        *rela_path = path + best_len + 1; /* plus 1 to skip the '/' */
    }

    return best;
}

static int __get_fildes(void)
{
    int i;

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        if (!fd_table[i].fd_used) {
            fd_table[i].fd_used = true;
            return i;
        }
    }

    return -1;
}

static struct file *__get_opened_file(const char *path)
{
    int           i;
    struct file * f;

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        f = fd_table[i].fd_file;
        if (f != NULL && strcmp(f->fl_path, path) == 0) {
            return f;
        }
    }

    return NULL;
}

static bool __has_opened_file(const char *path)
{
    int           i;
    size_t        len = strlen(path);
    struct file * f;

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        f = fd_table[i].fd_file;
        if (f != NULL && strncmp(f->fl_path, path, len) == 0) {
            return true;
        }
    }

    return false;
}

static struct file *__fd_2_file(int fd)
{
    if (fd < 0 || fd >= CONFIG_OPEN_MAX) {
        return NULL;
    }
    return fd_table[fd].fd_file;
}

static int __file_2_fd(struct file *f)
{
    int i;

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        if (fd_table[i].fd_file == f) {
            return i;
        }
    }

    return -1;
}

int vfs_open(const char *path, int oflag, ...)
{
    int                  fd  = -1;
    char               * new_path;
    const char         * rela_path;
    struct file        * fp;
    struct mount_point * mp;
    int                  mode = 0644;
    bool                 fs_locked = false;

    /* can not open dir */
    if (path == NULL || path[strlen(path) - 1] == '/') {
        return fd;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return fd;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    fs_locked = true;

    if (oflag & O_CREAT) {
        va_list valist;
        va_start(valist, oflag);
        mode = va_arg(valist, int); // get mode
        va_end(valist);
    }
    if (oflag & O_EXCL) {
        struct file *f = __get_opened_file(new_path);
        if (oflag & O_CREAT) {
            if (f) {
                goto EXIT_UNLOCK_FS;
            }
        } else {
            if (f) {
                f->fl_refs++;
                vmutex_unlock(&fs_lock);
                fs_locked = false;
                return __file_2_fd(f);
            } else {
                goto EXIT_UNLOCK_FS;
            }
        }
    }
    if ((fd = __get_fildes()) == -1) {
        goto EXIT_UNLOCK_FS;
    }

    mp = __path_resolve(new_path, &rela_path);
    if (mp == NULL || *rela_path == '\0') {
        goto EXIT_RELEASE_FD;
    }
    if (mp->mp_fs->fs_fops->open == NULL) {
        goto EXIT_RELEASE_FD;
    }

    /* lock mount point to prevent file deleted */
    if (vmutex_lock(&mp->mp_lock) != 0) {
        goto EXIT_RELEASE_FD;
    }

    vmutex_unlock(&fs_lock);
    fs_locked = false;

    fp = (struct file *)vmem_zalloc(sizeof(struct file));
    if (fp == NULL) {
        goto EXIT_UNLOCK_MP;
    }

    fp->fl_path  = new_path;
    fp->fl_flags = oflag;
    fp->fl_fops  = mp->mp_fs->fs_fops;
    fp->fl_mp    = mp;
    fp->fl_refs  = 1;
    fp->fl_data  = 0;

    fd_table[fd].fd_flags = oflag;

    if (fp->fl_fops->open(fp, rela_path, oflag, mode) != 0) {
        goto EXIT_FREE_FILE;
    }
    fd_table[fd].fd_file = fp;
    mp->mp_refs++;

    vmutex_unlock(&mp->mp_lock);

    return fd;

EXIT_FREE_FILE:
    vmem_free(fp);
EXIT_UNLOCK_MP:
    vmutex_unlock(&mp->mp_lock);
EXIT_RELEASE_FD:
    fd_table[fd].fd_used = false;
EXIT_UNLOCK_FS:
    if (fs_locked) {
        vmutex_unlock(&fs_lock);
    }
EXIT_FREE_PATH:
    vmem_free(new_path);
    return -1;
}

int vfs_close(int fd)
{
    struct file        * fp;
    struct mount_point * mp;
    int                  ret = -1;

    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->close == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (--fp->fl_refs != 0) {
        ret = 0;
        goto EXIT_UNLOCK_FS;
    }
    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) != 0) {
        goto EXIT_UNLOCK_FS;
    }
    vmutex_unlock(&fs_lock);

    ret = fp->fl_fops->close(fp);
    if (ret == 0) {
        memset(&fd_table[fd], 0, sizeof(struct fildes));
        if (mp->mp_refs > 0) {
            mp->mp_refs--;
        }
        vmem_free(fp->fl_path);
        vmem_free(fp);
    } else {
        fp->fl_refs++; /* restore */
    }

    vmutex_unlock(&mp->mp_lock);
    return ret;

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte)
{
    struct file        * fp;
    struct mount_point * mp;
    ssize_t              ret = -1;

    if (buf == NULL || nbyte == 0) {
        return ret;
    }
    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->read == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = fp->fl_fops->read(fp, buf, nbyte);
        vmutex_unlock(&mp->mp_lock);
        return ret;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

ssize_t vfs_write(int fd, const void *buf, size_t nbyte)
{
    struct file        * fp;
    struct mount_point * mp;
    ssize_t              ret = -1;

    if (buf == NULL || nbyte == 0) {
        return ret;
    }
    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->write == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = fp->fl_fops->write(fp, buf, nbyte);
        vmutex_unlock(&mp->mp_lock);
        return ret;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

/**
 * vfs_lseek - move the read/write file offset
 *
 * The lseek() function shall set the file offset for the open file description associated with the file descriptor
 * fildes, as follows:
 *      If whence is SEEK_SET, the file offset shall be set to offset bytes.
 *      If whence is SEEK_CUR, the file offset shall be set to its current location plus offset.
 *      If whence is SEEK_END, the file offset shall be set to the size of the file plus offset.
 */
off_t vfs_lseek(int fd, off_t offset, int whence)
{
    struct file        * fp;
    struct mount_point * mp;
    ssize_t              ret = -1;

    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->lseek == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = fp->fl_fops->lseek(fp, offset, whence);
        vmutex_unlock(&mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

int vfs_ioctl(int fd, int request, ...)
{
    struct file        * fp;
    struct mount_point * mp;
    ssize_t              ret = -1;
    va_list              valist;

    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->ioctl == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);

        va_start(valist, request);
        ret = fp->fl_fops->ioctl(fp, request, valist);
        va_end(valist);

        vmutex_unlock(&mp->mp_lock);
        return ret;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

int vfs_sync(int fd)
{
    struct file        * fp;
    struct mount_point * mp;
    ssize_t              ret = -1;

    if ((fp = __fd_2_file(fd)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        return ret;
    }
    if (fp->fl_fops->sync == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->fl_mp;
    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = fp->fl_fops->sync(fp);
        vmutex_unlock(&mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
    return ret;
}

int vfs_stat(const char *path, struct stat *st)
{
    int                  ret  = -1;
    struct mount_point * mp;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL || st == NULL) {
        return ret;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    if ((mp = __path_resolve(new_path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (mp->mp_fs->fs_fops->stat == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = mp->mp_fs->fs_fops->stat(mp, rela_path, st);
        vmutex_unlock(&mp->mp_lock);
        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return ret;
}

int vfs_unlink(const char *path)
{
    int                  ret  = -1;
    struct mount_point * mp;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL) {
        return ret;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    /* file is opened */
    if (__get_opened_file(new_path) != NULL) {
        goto EXIT_UNLOCK_FS;
    }

    /* if there is any file under the dir of path */
    if (__has_opened_file(new_path)) {
        goto EXIT_UNLOCK_FS;
    }
    if ((mp = __path_resolve(new_path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->unlink == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = mp->mp_fs->fs_fops->unlink(mp, rela_path);
        vmutex_unlock(&mp->mp_lock);
        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return ret;
}

int vfs_rename(const char *from, const char *to)
{
    int                  ret = -1;
    struct mount_point * mp;
    char               * old_path;
    char               * new_path;
    const char         * rela_path_old;
    const char         * rela_path_new;

    if (from == NULL || to == NULL) {
        return ret;
    }
    if ((old_path = __get_new_path(from)) == NULL) {
        return ret;
    }
    if ((new_path = __get_new_path(to)) == NULL) {
        goto EXIT_FREE_OLD_PATH;
    }

    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_NEW_PATH;
    }

    /* old or new is already opened file */
    if (__get_opened_file(old_path) != NULL ||
        __get_opened_file(new_path) != NULL ||
        __has_opened_file(old_path)) {
        goto EXIT_UNLOCK_FS;
    }

    if ((mp = __path_resolve(old_path, &rela_path_old)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (mp != __path_resolve(new_path, &rela_path_new)) {
        goto EXIT_UNLOCK_FS;
    }
    if (mp->mp_fs->fs_fops->rename == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = mp->mp_fs->fs_fops->rename(mp, rela_path_old, rela_path_new);
        vmutex_unlock(&mp->mp_lock);
        goto EXIT_FREE_NEW_PATH;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_NEW_PATH:
    vmem_free(new_path);
EXIT_FREE_OLD_PATH:
    vmem_free(old_path);
    return ret;
}

struct dir *vfs_opendir(const char *path)
{
    int                  ret = -1;
    struct dir         * dp  = NULL;
    struct mount_point * mp;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL) {
        return NULL;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return NULL;
    }
    dp = (struct dir *)vmem_zalloc(sizeof(struct dir));
    if (dp == NULL) {
        goto EXIT_FREE_PATH;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_DIR;
    }
    if ((mp = __path_resolve(new_path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (mp->mp_fs->fs_fops->opendir == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        dp->dr_mp = mp; // before opendir
        ret = mp->mp_fs->fs_fops->opendir(mp, dp, rela_path);
        vmutex_unlock(&mp->mp_lock);
        if (ret != 0) {
            goto EXIT_FREE_DIR;
        }
        vmem_free(new_path);
        mp->mp_refs++;

        return dp;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_DIR:
    vmem_free(dp);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return NULL;
}

struct dirent *vfs_readdir(struct dir *dp)
{
    struct mount_point * mp;
    struct dirent      * entry = NULL;

    if (dp == NULL) {
        return NULL;
    }
    if ((mp = dp->dr_mp) == NULL) {
        return NULL;
    }
    if (vmutex_lock(&mp->mp_lock) != 0) {
        return NULL;
    }
    if (mp->mp_fs->fs_fops->readdir != NULL &&
        mp->mp_fs->fs_fops->readdir(mp, dp, &dp->dr_dent) == 0) {
        entry = &dp->dr_dent;
    }
    vmutex_unlock(&mp->mp_lock);
    return entry;
}

int vfs_closedir(struct dir *dp)
{
    int                  ret = 0;
    struct mount_point * mp;

    if (dp == NULL) {
        return -1;
    }
    if ((mp = dp->dr_mp) == NULL) {
        return -1;
    }
    if (vmutex_lock(&mp->mp_lock) != 0) {
        return -1;
    }
    if (mp->mp_fs->fs_fops->closedir != NULL) {
        ret = mp->mp_fs->fs_fops->closedir(mp, dp);
    }
    if (ret == 0) {
        if (mp->mp_refs > 0) {
            mp->mp_refs--;
        }
        vmem_free(dp);
    }
    vmutex_unlock(&mp->mp_lock);

    return 0;
}

int vfs_mkdir(const char *path, int mode)
{
    int                  ret = -1;
    struct mount_point * mp;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL) {
        return -1;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return -1;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    if ((mp = __path_resolve((const char *)new_path, &rela_path)) == NULL ||
        *rela_path == '\0') {
        goto EXIT_UNLOCK_FS;
    }
    if (vmutex_lock(&mp->mp_lock) != 0) {
        goto EXIT_UNLOCK_FS;
    }
    vmutex_unlock(&fs_lock);

    if (mp->mp_fs->fs_fops->mkdir != NULL) {
        ret = mp->mp_fs->fs_fops->mkdir(mp, rela_path, mode);
    }
    vmutex_unlock(&mp->mp_lock);
    goto EXIT_FREE_PATH;

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return ret;
}

int vfs_rmdir(const char *path)
{
    int                  ret = -1;
    struct mount_point * mp;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL) {
        return ret;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return ret;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    /* file is opened */
    if (__get_opened_file(new_path) != NULL) {
        goto EXIT_UNLOCK_FS;
    }
    /* if there is any file under the dir of path */
    if (__has_opened_file(new_path)) {
        goto EXIT_UNLOCK_FS;
    }
    if ((mp = __path_resolve(new_path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (mp->mp_fs->fs_fops->rmdir == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vmutex_lock(&mp->mp_lock) == 0) {
        vmutex_unlock(&fs_lock);
        ret = mp->mp_fs->fs_fops->rmdir(mp, rela_path);
        vmutex_unlock(&mp->mp_lock);
        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return ret;
}

/**
 * @brief virtual file-system switch mount interface
 *
 * @param fs_name [IN] the file-system name
 * @param mp_path [IN] the mount point path name, begin with '/'
 * @param ...     [IN] more variable is passed depend on the filesystem type
 *
 * @return 0 on success, negtive value on error
 */
int vfs_mount(const char *fs_name, const char *mp_path, ...)
{
    int                  ret;
    struct file_system * fs;
    struct mount_point * mp;
    va_list              valist;
    char               * new_path;
    const char         * rela_path;

    if (fs_name == NULL || mp_path == NULL || mp_path[0] != '/') {
        return -1;
    }
    if ((new_path = __get_new_path(mp_path)) == NULL) {
        return -1;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    if ((fs = __get_fs_by_name(fs_name)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    /* the mp_path is exist */
    if (__path_resolve(new_path, &rela_path) != NULL && *rela_path == '\0') {
        goto EXIT_UNLOCK_FS;
    }
    if (fs->fs_fops->mount == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = (struct mount_point *)vmem_zalloc(sizeof(struct mount_point));
    if (mp == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    mp->mp_path = new_path;
    mp->mp_fs   = fs;
    mp->mp_refs = 0;

    if (vmutex_init(&mp->mp_lock) != 0) {
        goto EXIT_FREE_MP;
    }

    va_start(valist, mp_path);
    ret = fs->fs_fops->mount(mp, valist);
    va_end(valist);
    if (ret != 0) {
        goto EXIT_FREE_MP;
    }
    vdlist_add_tail(&mp_head, &mp->mp_node);
    fs->fs_refs++;
    vmutex_unlock(&fs_lock);

    return 0;

EXIT_FREE_MP:
    vmem_free(mp);
EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return -1;
}

int vfs_unmount(const char *path, ...)
{
    int                  ret;
    struct mount_point * mp;
    va_list              valist;
    char               * new_path;
    const char         * rela_path;

    if (path == NULL) {
        return -1;
    }
    if ((new_path = __get_new_path(path)) == NULL) {
        return -1;
    }
    if (vmutex_lock(&fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }

    mp = __path_resolve(new_path, &rela_path);
    if (mp == NULL || *rela_path != '\0') {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_refs != 0) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->unmount != NULL) {
        va_start(valist, path);
        ret = mp->mp_fs->fs_fops->unmount(mp, valist);
        va_end(valist);
        if (ret != 0) {
            goto EXIT_UNLOCK_FS;
        }
    }
    vdlist_del(&mp->mp_node);
    if (mp->mp_fs->fs_refs > 0) {
        mp->mp_fs->fs_refs--;
    }
    vmutex_destroy(&mp->mp_lock);
    vmem_free(mp->mp_path);
    vmem_free(mp);
    vmem_free(new_path);

    vmutex_unlock(&fs_lock);

    return 0;

EXIT_UNLOCK_FS:
    vmutex_unlock(&fs_lock);
EXIT_FREE_PATH:
    vmem_free(new_path);
    return -1;
}

int vfs_init(void)
{
    if (vmutex_init(&fs_lock) != 0) {
        return -1;
    }
    return 0;
}

int vfs_exit(void)
{
    if (vdlist_empty(&fs_head)) {
        return vmutex_destroy(&fs_lock);
    }
    return 0;
}

VMODULE_DEF(VMODULE_LIB, vfs_init, vfs_exit);

int vfs_register(struct file_system *fs)
{
    int ret = -1;

    if (fs                  == NULL ||
        fs->fs_name         == NULL ||
        fs->fs_fops         == NULL ||
        fs->fs_fops->open   == NULL ||
        fs->fs_fops->read   == NULL ||
        fs->fs_fops->write  == NULL ||
        fs->fs_fops->stat   == NULL ||
        fs->fs_fops->mount  == NULL) {
        return -1;
    }
    /* check fs name */
    if (!__fs_name_valid(fs->fs_name)) {
        return -1;
    }

    if (vmutex_lock(&fs_lock) != 0) {
        return -1;
    }
    if (__get_fs_by_name(fs->fs_name) == NULL) {
        fs->fs_refs = 0;
        vdlist_add_tail(&fs_head, &fs->fs_node);
        ret = 0;
    }
    vmutex_unlock(&fs_lock);

    return ret;
}

int vfs_unregister(struct file_system *fs)
{
    if (fs == NULL) {
        return -1;
    }

    if (vmutex_lock(&fs_lock) != 0) {
        return -1;
    }
    if (fs->fs_refs == 0) {
        vdlist_del(&fs->fs_node);
    }
    vmutex_unlock(&fs_lock);

    return 0;
}
