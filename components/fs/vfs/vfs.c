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

#include "vfs.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONFIG_OPEN_MAX
#define CONFIG_OPEN_MAX 16
#endif

#define FILE_STATUS_NOT_USED 0
#define FILE_STATUS_INITING 1
#define FILE_STATUS_READY 2
#define FILE_STATUS_CLOSING 3

static struct file files[CONFIG_OPEN_MAX];
static vmutex_t fs_lock = V_MUTEX_INVALID;
static struct file_system *file_systems = NULL;
static struct mnt *mount_points = NULL;

static inline int __file_2_fd(struct file *file) { return file - files; }

static inline struct file *__fd_2_file(int fd) { return &files[fd]; }

static bool __is_fs_name_valid(const char *fs_name)
{
    int ch;
    const char *name = fs_name;

    while ((ch = *name++) != '\0') {
        if (!islower(ch | 0x20) && !isdigit(ch)) {
            return false;
        }
    }

    return (name != fs_name);
}

static struct file_system *__get_file_system(const char *name)
{
    struct file_system *fs = file_systems;

    while (fs != NULL) {
        if (strcmp(name, fs->fs_name) == 0) {
            break;
        }
        fs = fs->fs_next;
    }

    return fs;
}

static void __take_file_system(const struct file_system *fs)
{
    struct file_system *cur = file_systems;
    struct file_system *prev = NULL;

    while (cur != NULL) {
        if (cur == fs) {
            break;
        }
        prev = cur;
        cur = cur->fs_next;
    }
    // not found
    if (cur == NULL) {
        return;
    }
    // take it
    if (cur == file_systems) {
        file_systems = cur->fs_next;
    } else if (prev != NULL) {
        prev->fs_next = cur->fs_next;
    }
}

static char *__get_new_path(const char *path)
{
    char *p = (char *)vos_malloc(strlen(path) + 1);
    char *t = p;
    char ch;
    char last = '\0';

    if (p == NULL) {
        return NULL;
    }

    while ((ch = *path++) != '\0') {
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

static struct mnt *__path_resolve(const char *path, const char **rela_path)
{
    struct mnt *mp = mount_points;
    struct mnt *best = NULL;
    size_t best_len = 0;

    while (mp != NULL) {
        size_t len; /* mount point path string length */
        const char *sub;

        sub = strstr(path, mp->mp_path);
        if (sub == NULL || sub != path) {
            mp = mp->mp_next;
            continue;
        }
        len = strlen(mp->mp_path);

        if (path[len] == '\0') {
            best_len = len - 1; /* so, best_len + 1 == len */
            best = mp;
            break;
        }

        if ((len > best_len) && (path[len] == '/')) {
            best_len = len;
            best = mp;
        }
        mp = mp->mp_next;
    }

    if (rela_path != NULL) {
        *rela_path = path + best_len + 1; /* plush 1 to skip the '/' */
    }

    return best;
}

static int __get_fildes(const char *path)
{
    int i;

    /* protected by fs_mutex */

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        if (files[i].f_status == FILE_STATUS_NOT_USED) {
            files[i].f_status = FILE_STATUS_INITING;
            return i;
        }
    }

    return -1;
}

static struct file *__get_opened_file(const char *path)
{
    int i;

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        if (files[i].f_path != NULL && strcmp(files[i].f_path, path) == 0) {
            return &files[i];
        }
    }

    return NULL;
}

static bool __has_opened_file(const char *path)
{
    int i;
    size_t len = strlen(path);

    for (i = 0; i < CONFIG_OPEN_MAX; ++i) {
        if (files[i].f_path != NULL && strncmp(files[i].f_path, path, len) == 0) {
            return true;
        }
    }

    return false;
}

static struct file *__get_file(int fd)
{
    struct file *file = NULL;
    if (fd < 0 || fd >= CONFIG_OPEN_MAX) {
        return file;
    }
    file = __fd_2_file(fd);
    if (file->f_status == FILE_STATUS_READY) {
        return file;
    }
    return NULL;
}

int vfs_open(const char *o_path, int flags, ...)
{
    int ret = -1;
    int fd = -1;
    char *path;
    const char *rela_path;
    struct file *fp;
    struct mnt *mp;
    int mode = 0644;

    if (o_path == NULL) {
        return fd;
    }

    if (flags & O_CREAT) {
        va_list valist;
        va_start(valist, flags);
        mode = va_arg(valist, int); // get mode
        va_end(valist);
    }

    /* can not open dir */
    if (o_path[strlen(o_path) - 1] == '/') {
        return fd;
    }
    path = __get_new_path(o_path);
    if (path == NULL) {
        return fd;
    }

    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }

    if ((fd = __get_fildes(path)) == -1) {
        goto EXIT_UNLOCK_FS;
    }

    if ((flags & O_EXCL) && __get_opened_file(path) != NULL) {
        goto EXIT_RELEASE_FD;
    }

    if ((mp = __path_resolve((const char *)path, &rela_path)) == NULL || *rela_path == '\0') {
        goto EXIT_RELEASE_FD;
    }

    /* lock mount point to prevent file deleted */
    if (vos_mutex_lock(mp->mp_lock) != 0) {
        goto EXIT_RELEASE_FD;
    }

    vos_mutex_unlock(fs_lock);

    fp = __fd_2_file(fd);

    fp->f_path = path;
    fp->f_flags = flags;
    fp->f_fops = mp->mp_fs->fs_fops;
    fp->f_mp = mp;
    fp->f_refs = 1;
    fp->f_data = NULL;

    ret = fp->f_fops->open(fp, rela_path, flags, mode);
    if (ret == 0) {
        mp->mp_refs++;
        fp->f_status = FILE_STATUS_READY;
    } else {
        goto EXIT_UNLOCK_MD;
    }

    vos_mutex_unlock(mp->mp_lock);

    return fd;

EXIT_UNLOCK_MD:
    vos_mutex_unlock(mp->mp_lock);
EXIT_RELEASE_FD:
    files[fd].f_status = FILE_STATUS_NOT_USED;
EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return -1;
}

int vfs_close(int fd)
{
    struct file *fp;
    struct mnt *mp;
    int ret = -1;

    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }
    if (--fp->f_refs != 0) {
        ret = 0;
        goto EXIT_UNLOCK_FS;
    }
    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) != 0) {
        goto EXIT_UNLOCK_FS;
    }
    vos_mutex_unlock(fs_lock);
    if ((ret = fp->f_fops->close(fp)) == 0) {
        fp->f_status = FILE_STATUS_NOT_USED;
        vos_free(fp->f_path);
        fp->f_path = NULL;
        mp->mp_refs--;
    }
    vos_mutex_unlock(mp->mp_lock);
    return ret;

EXIT_UNLOCK_FS:
    vos_mutex_unlock(mp->mp_lock);
    return ret;
}

ssize_t vfs_read(int fd, void *buf, size_t bytes)
{
    struct file *fp;
    struct mnt *mp;
    ssize_t ret = -1;

    if (buf == NULL || bytes == 0) {
        return ret;
    }
    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }

    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = fp->f_fops->read(fp, buf, bytes);
        vos_mutex_unlock(mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return ret;
}

ssize_t vfs_write(int fd, const char *buf, size_t bytes)
{
    struct file *fp;
    struct mnt *mp;
    ssize_t ret = -1;

    if (buf == NULL || bytes == 0) {
        return ret;
    }
    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }

    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = fp->f_fops->write(fp, buf, bytes);
        vos_mutex_unlock(mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return ret;
}

off_t vfs_lseek(int fd, off_t off, int whence)
{
    struct file *fp;
    struct mnt *mp;
    off_t ret = -1;

    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }
    if (fp->f_fops->lseek == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = fp->f_fops->lseek(fp, off, whence);
        vos_mutex_unlock(mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return ret;
}

int vfs_ioctl(int fd, int request, ...)
{
    struct file *fp;
    struct mnt *mp;
    int ret = -1;
    unsigned long arg;
    va_list valist;

    va_start(valist, request);
    arg = va_arg(valist, unsigned long);
    va_end(valist);

    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }
    if (fp->f_fops->ioctl == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = fp->f_fops->ioctl(fp, request, arg);
        vos_mutex_unlock(mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return ret;
}

int vfs_sync(int fd)
{
    struct file *fp;
    struct mnt *mp;
    int ret = -1;

    if ((fp = __get_file(fd)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }
    if (fp->f_fops->sync == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    mp = fp->f_mp;
    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = fp->f_fops->sync(fp);
        vos_mutex_unlock(mp->mp_lock);

        return ret;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return ret;
}

int vfs_stat(const char *s_path, struct stat *stat)
{
    struct mnt *mp;
    const char *rela_path;
    int ret = -1;
    char *path = NULL;

    if (s_path == NULL || stat == NULL) {
        return ret;
    }
    if ((path = __get_new_path(s_path)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    if ((mp = __path_resolve(path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->stat == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = mp->mp_fs->fs_fops->stat(mp, rela_path, stat);
        vos_mutex_unlock(mp->mp_lock);

        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return ret;
}

int vfs_unlink(const char *u_path)
{
    struct mnt *mp;
    const char *rela_path;
    int ret = -1;
    char *path = NULL;

    if (u_path == NULL) {
        return ret;
    }
    if ((path = __get_new_path(u_path)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    /* file is opened */
    if (__get_opened_file(path) != NULL) {
        goto EXIT_UNLOCK_FS;
    }

    /* if there is any file under the dir of path */
    if (__has_opened_file(path)) {
        goto EXIT_UNLOCK_FS;
    }
    if ((mp = __path_resolve(path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->unlink == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = mp->mp_fs->fs_fops->unlink(mp, rela_path);
        vos_mutex_unlock(mp->mp_lock);

        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return ret;
}

int vfs_rename(const char *oldpath, const char *newpath)
{
    struct mnt *mp;
    char *old_path;
    char *new_path;
    const char *rela_path_old;
    const char *rela_path_new;
    int ret = -1;

    if (oldpath == NULL || newpath == NULL) {
        return ret;
    }

    if ((old_path = __get_new_path(oldpath)) == NULL) {
        return ret;
    }

    if ((new_path = __get_new_path(newpath)) == NULL) {
        goto EXIT_FREE_OLD_PATH;
    }

    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_NEW_PATH;
    }

    /* old or new is already opened file */
    if (__get_opened_file(old_path) != NULL || __get_opened_file(new_path) != NULL || __has_opened_file(old_path)) {
        goto EXIT_UNLOCK_FS;
    }

    if ((mp = __path_resolve((const char *)old_path, &rela_path_old)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp != __path_resolve((const char *)new_path, &rela_path_new)) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->rename == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = mp->mp_fs->fs_fops->rename(mp, rela_path_old, rela_path_new);
        vos_mutex_unlock(mp->mp_lock);

        goto EXIT_FREE_NEW_PATH;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_NEW_PATH:
    vos_free(new_path);
EXIT_FREE_OLD_PATH:
    vos_free(old_path);
    return ret;
}

struct dir *vfs_opendir(const char *o_path)
{
    struct mnt *mp;
    const char *rela_path = NULL;
    struct dir *dir = NULL;
    int ret = -1;
    char *path;

    if (o_path == NULL) {
        return NULL;
    }
    if ((path = __get_new_path(o_path)) == NULL) {
        return NULL;
    }

    if ((dir = (struct dir *)vos_malloc(sizeof(struct dir))) == NULL) {
        goto EXIT_FREE_PATH;
    }

    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_DIR;
    }
    if ((mp = __path_resolve(path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->opendir == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        dir->d_mp = mp;
        ret = mp->mp_fs->fs_fops->opendir(dir, rela_path);
        vos_mutex_unlock(mp->mp_lock);
        if (ret != 0) {
            goto EXIT_FREE_DIR;
        }
        vos_free(path);
        mp->mp_refs++;

        return dir;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_DIR:
    vos_free(dir);
EXIT_FREE_PATH:
    vos_free(path);
    return NULL;
}

struct dirent *vfs_readdir(struct dir *dir)
{
    struct mnt *mp;
    struct dirent *entry = NULL;

    if (dir == NULL) {
        return NULL;
    }
    if ((mp = dir->d_mp) == NULL) {
        return NULL;
    }
    if (vos_mutex_lock(mp->mp_lock) != 0) {
        return NULL;
    }
    if (mp->mp_fs->fs_fops->readdir != NULL && mp->mp_fs->fs_fops->readdir(dir, &dir->d_dent) == 0) {
        entry = &dir->d_dent;
    }
    vos_mutex_unlock(mp->mp_lock);
    return entry;
}

int vfs_closedir(struct dir *dir)
{
    struct mnt *mp;

    if (dir == NULL) {
        return -1;
    }
    if ((mp = dir->d_mp) == NULL) {
        return -1;
    }
    if (vos_mutex_lock(mp->mp_lock) != 0) {
        return -1;
    }
    if (mp->mp_fs->fs_fops->closedir != NULL) {
        mp->mp_fs->fs_fops->closedir(dir);
    }
    vos_free(dir);
    mp->mp_refs--;
    vos_mutex_unlock(mp->mp_lock);

    return 0;
}

int vfs_mkdir(const char *c_path, int mode)
{
    struct mnt *mp;
    char *path;
    const char *rela_path;
    int ret = -1;

    if (c_path == NULL) {
        return -1;
    }
    if ((path = __get_new_path(c_path)) == NULL) {
        return -1;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    if ((mp = __path_resolve((const char *)path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    if (*rela_path == '\0') { /* file/dir existed */
        goto EXIT_UNLOCK_FS;
    }
    if (vos_mutex_lock(mp->mp_lock) != 0) {
        goto EXIT_UNLOCK_FS;
    }
    vos_mutex_unlock(fs_lock);

    if (mp->mp_fs->fs_fops->mkdir != NULL) {
        ret = mp->mp_fs->fs_fops->mkdir(mp, rela_path, mode);
    }
    vos_mutex_unlock(mp->mp_lock);
    goto EXIT_FREE_PATH;

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return ret;
}

int vfs_rmdir(const char *c_path)
{
    struct mnt *mp;
    const char *rela_path;
    int ret = -1;
    char *path = NULL;

    if (c_path == NULL) {
        return ret;
    }
    if ((path = __get_new_path(c_path)) == NULL) {
        return ret;
    }
    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }
    /* file is opened */
    if (__get_opened_file(path) != NULL) {
        goto EXIT_UNLOCK_FS;
    }

    /* if there is any file under the dir of path */
    if (__has_opened_file(path)) {
        goto EXIT_UNLOCK_FS;
    }
    if ((mp = __path_resolve(path, &rela_path)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (mp->mp_fs->fs_fops->rmdir == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (vos_mutex_lock(mp->mp_lock) == 0) {
        vos_mutex_unlock(fs_lock);
        ret = mp->mp_fs->fs_fops->rmdir(mp, rela_path);
        vos_mutex_unlock(mp->mp_lock);

        goto EXIT_FREE_PATH;
    }

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return ret;
}

int vfs_mount(const char *fs_name, const char *mp_path, void *data)
{
    int ret;
    struct file_system *fs;
    struct mnt *mp;
    char *path;
    const char *temp;

    if (fs_name == NULL || mp_path == NULL || mp_path[0] != '/') {
        return -1;
    }

    if ((path = __get_new_path(mp_path)) == NULL) {
        return -1;
    }

    if (vos_mutex_lock(fs_lock) != 0) {
        goto EXIT_FREE_PATH;
    }

    if ((fs = __get_file_system(fs_name)) == NULL) {
        goto EXIT_UNLOCK_FS;
    }

    if (__path_resolve((const char *)path, &temp) != NULL && *temp == '\0') {
        goto EXIT_UNLOCK_FS;
    }

    mp = (struct mnt *)vos_malloc(sizeof(struct mnt));
    if (mp == NULL) {
        goto EXIT_UNLOCK_FS;
    }
    mp->mp_path = path;
    mp->mp_fs = fs;
    mp->mp_refs = 0;
    mp->mp_data = data;

    if (vos_mutex_init(&mp->mp_lock) != 0) {
        goto EXIT_FREE_MP;
    }

    mp->mp_next = mount_points;
    mount_points = mp;

    fs->fs_refs++;

    vos_mutex_unlock(fs_lock);

    return 0;

EXIT_FREE_MP:
    vos_free(mp);
EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
EXIT_FREE_PATH:
    vos_free(path);
    return -1;
}

int vfs_unmount(const char *path)
{
    int ret = -1;
    struct mnt *mp;
    struct mnt *prev;

    if (path == NULL) {
        return ret;
    }

    if ((ret = vos_mutex_lock(fs_lock)) != 0) {
        return ret;
    }

    mp = __path_resolve(path, NULL);
    if (mp == NULL || mp->mp_refs != 0) {
        goto EXIT_UNLOCK_FS;
    }

    if (mount_points == mp) {
        mount_points = mp->mp_next;
    } else {
        for (prev = mount_points; prev != NULL; prev = prev->mp_next) {
            if (prev->mp_next != mp) {
                continue;
            }

            prev->mp_next = mp->mp_next;
            break;
        }
    }

    vos_mutex_unlock(fs_lock);

    mp->mp_fs->fs_refs--;

    vos_free(mp->mp_path);
    vos_free(mp);

    return 0;

EXIT_UNLOCK_FS:
    vos_mutex_unlock(fs_lock);
    return -1;
}

int vfs_init(void)
{
    if (fs_lock != V_MUTEX_INVALID) {
        return 0;
    }
    if (vos_mutex_init(&fs_lock) == 0) {
        return 0;
    }
    return -1;
}

int vfs_register(struct file_system *fs)
{
    int ret = -1;

    if (fs == NULL || fs->fs_name == NULL || fs->fs_fops == NULL) {
        return ret;
    }

    if (fs->fs_fops->open == NULL || fs->fs_fops->read == NULL || fs->fs_fops->write == NULL ||
        fs->fs_fops->close == NULL) {
        return ret;
    }

    if (!__is_fs_name_valid(fs->fs_name)) {
        return -1;
    }
    fs->fs_refs = 0;

    vos_mutex_lock(fs_lock);
    if (__get_file_system(fs->fs_name) == NULL) {
        fs->fs_next = file_systems;
        file_systems = fs;
        ret = 0;
    }
    vos_mutex_unlock(fs_lock);

    return ret;
}

int vfs_unregister(struct file_system *fs)
{
    int ret = -1;

    if (fs == NULL) {
        return ret;
    }

    if (vos_mutex_lock(fs_lock) != 0) {
        return ret;
    }
    if (fs->fs_refs == 0) {
        __take_file_system(fs);
    }
    vos_mutex_unlock(fs_lock);

    return ret;
}
