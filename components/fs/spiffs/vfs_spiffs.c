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

#include "vfs_spiffs.h"
#include "vfs.h"

#include "spiffs_nucleus.h"


#ifndef SAFE_FREE
#define SAFE_FREE(p)  do {  if (p != NULL) {  free(p); p = NULL;  } } while(0)
#endif

typedef struct
{
    spiffs  *fs;
    u8_t    *wbuf;  // secondary work buffer, size of a logical page
    u8_t    *fds;   // file descriptor memory area
    u8_t    *cache; // cache memory
} spiffs_cb;

static int spiffs_flags_get(int oflags)
{
    int flags = 0;

    switch (oflags & O_ACCMODE)
    {
    case O_RDONLY:
        flags |= SPIFFS_O_RDONLY;
        break;
    case O_WRONLY:
        flags |= SPIFFS_O_WRONLY;
        break;
    case O_RDWR:
        flags |= SPIFFS_O_RDWR;
        break;
    default:
        break;
    }

    if (oflags & O_CREAT) {
        flags |= SPIFFS_O_CREAT;
    }

    if (oflags & O_EXCL) {
        flags |= SPIFFS_O_EXCL;
    }

    if (oflags & O_TRUNC) {
        flags |= SPIFFS_O_TRUNC;
    }

    if (oflags & O_APPEND) {
        flags |= SPIFFS_O_CREAT | SPIFFS_O_APPEND;
    }

    return flags;
}

static spiffs_file spifd_from_file(struct file *file)
{
    return (spiffs_file)(uintptr_t)file->f_data;
}

static int spiffs_op_open(struct file *file, const char *path, int flags, int mode)
{
    spiffs              *fs = (spiffs *)file->f_mp->mp_data;
    spiffs_file          s_file;

    s_file = SPIFFS_open(fs, path, spiffs_flags_get(flags), mode);
    if (s_file < SPIFFS_OK) {
        vlog_error("SPIFFS_open() failed, fd=%d", s_file);
        return -1;
    }

    file->f_data = (void *)(uintptr_t)s_file;

    return 0;
}

static int spiffs_op_close(struct file *file)
{
    spiffs_file  s_file = spifd_from_file(file);
    spiffs      *fs     = (spiffs *)file->f_mp->mp_data;

    return (SPIFFS_close(fs, s_file) == SPIFFS_OK) ? 0 : -1;
}

static ssize_t spiffs_op_read(struct file *file, char *buff, size_t bytes)
{
    spiffs_file  s_file = spifd_from_file(file);
    spiffs      *fs     = (spiffs *)file->f_mp->mp_data;

    return (ssize_t)SPIFFS_read(fs, s_file, buff, bytes);
}

static ssize_t spiffs_op_write(struct file *file, const char *buff, size_t bytes)
{
    spiffs_file  s_file = spifd_from_file(file);
    spiffs      *fs     = (spiffs *)file->f_mp->mp_data;

    return (ssize_t)SPIFFS_write(fs, s_file, (void *)buff, bytes);
}

static off_t spiffs_op_lseek(struct file *file, off_t off, int whence)
{
    spiffs_file  s_file = spifd_from_file(file);
    spiffs      *fs     = (spiffs *)file->f_mp->mp_data;

    return (off_t)SPIFFS_lseek(fs, s_file, off, whence);
}

int spiffs_op_stat(struct mnt *mp, const char *path, struct stat *stat)
{
    spiffs_file  s_file;;
    spiffs      *fs = (spiffs *)mp->mp_data;
    spiffs_stat  s;

    s_file = SPIFFS_open(fs, path, spiffs_flags_get(O_RDONLY), 0);
    if (s_file < SPIFFS_OK) {
        vlog_error("SPIFFS_open() failed, fd=%d", s_file);
        return -1;
    }

    memset(&s, 0, sizeof(s));
    s32_t ret = SPIFFS_fstat(fs, s_file, &s);
    if (ret != SPIFFS_OK) {
        vlog_error("SPIFFS_fstat() failed, ret=%d", ret);
        SPIFFS_close(fs, s_file);
        return -1;
    }
    stat->st_size = s.size;
    SPIFFS_close(fs, s_file);
    return 0;
}

static int spiffs_op_unlink(struct mnt *mp, const char *path)
{
    s32_t ret = SPIFFS_remove((spiffs *)mp->mp_data, path);
    if (ret != SPIFFS_OK) {
        vlog_error("SPIFFS_remove() failed, ret=%d", ret);
        return -1;
    }
    return 0;
}

static int spiffs_op_rename(struct mnt *mp, const char *path_old, const char *path_new)
{
    s32_t ret = SPIFFS_rename((spiffs *)mp->mp_data, path_old, path_new);
    if (ret != SPIFFS_OK) {
        vlog_error("SPIFFS_rename() failed, ret=%d", ret);
        return -1;
    }
    return 0;
}

static int spiffs_op_sync(struct file *file)
{
    spiffs_file  s_file = spifd_from_file(file);
    spiffs      *fs     = (spiffs *)file->f_mp->mp_data;

    s32_t ret = SPIFFS_fflush(fs, s_file);
    if (ret != SPIFFS_OK) {
        vlog_error("SPIFFS_fflush() failed, ret=%d", ret);
        return -1;
    }
    return 0;
}

static int spiffs_op_opendir(struct dir *dir, const char *path)
{
    spiffs      *fs     = (spiffs *)dir->d_mp->mp_data;
    spiffs_DIR  *sdir   = NULL;

    sdir = (spiffs_DIR *)malloc(sizeof(spiffs_DIR));
    if (sdir == NULL) {
        vlog_error("fail to malloc memory in SPIFFS, <malloc.c> is needed,"
                  "make sure it is added");
        return -1;
    }

    dir->d_data = (void *)SPIFFS_opendir(fs, path, sdir);
    if (dir->d_data == 0) {
        free(sdir);
        return -1;
    }

    return 0;
}

int spiffs_op_readdir(struct dir *dir, struct dirent *dent)
{
    struct spiffs_dirent entry;
    int                  len;

    if (SPIFFS_readdir((spiffs_DIR *)dir->d_data, &entry) == NULL) {
        return -1;
    }

    len = min(sizeof(entry.name), sizeof(dent->d_name)) - 1;
    strncpy((char *)dent->d_name, (const char *)entry.name, len);
    dent->d_name[len] = '\0';
    dent->d_size = entry.size;

    switch (entry.type) {
    case SPIFFS_TYPE_FILE:
        dent->d_type = DT_REG;
        break;
    case SPIFFS_TYPE_DIR:
        dent->d_type = DT_DIR;
        break;
    case SPIFFS_TYPE_HARD_LINK:
    case SPIFFS_TYPE_SOFT_LINK:
        dent->d_type = DT_LNK;
        break;
    default:
        dent->d_type = DT_UNKNOWN;
        break;
    }

    return 0;
}

static int spiffs_op_closedir(struct dir *dir)
{
    spiffs_DIR *sdir = (spiffs_DIR *)dir->d_data;

    s32_t ret = SPIFFS_closedir(sdir);
    if (ret != SPIFFS_OK) {
        vlog_error("SPIFFS_closedir() failed, ret=%d", ret);
        return -1;
    }
    free(sdir);
    dir->d_data = NULL;

    return 0;
}

static struct file_ops spiffs_ops =
{
    spiffs_op_open,
    spiffs_op_close,
    spiffs_op_read,
    spiffs_op_write,
    spiffs_op_lseek,
    spiffs_op_stat,
    spiffs_op_unlink,
    spiffs_op_rename,
    NULL,               /* ioctl not supported for now */
    spiffs_op_sync,
    spiffs_op_opendir,
    spiffs_op_readdir,
    spiffs_op_closedir,
    NULL,               /* spiffs do not support mkdir */
    NULL                /* spiffs do not support rmdir */
};

static struct file_system spiffs_fs =
{
    "spiffs",
    &spiffs_ops,
    NULL,
    0
};

static spiffs_cb s_cb;

int spiffs_init(void)
{
    static int spiffs_inited = 0;

    if (spiffs_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&spiffs_fs) != 0) {
        vlog_error("failed to register spiffs!");
        return -1;
    }

    spiffs_inited = 1;

    vlog_info("register spiffs done!");

    return 0;
}

int spiffs_mount(const char *path, spiffs_config *config)
{
    spiffs         *fs;
    u8_t           *wbuf;
    u8_t           *fds;
    u8_t           *cache;
    int             ret = -1;

    if (path == NULL || config == NULL) {
        return -1;
    }

#define LOS_SPIFFS_FD_SIZE      (sizeof(spiffs_fd) << 3)
#define LOS_SPIFFS_CACHE_SIZE   (((config->log_page_size + 32) << 2) + 40)

    fs    = (spiffs *) malloc (sizeof (spiffs));
    wbuf  = (u8_t *)   malloc (config->log_page_size << 1);
    fds   = (u8_t *)   malloc (LOS_SPIFFS_FD_SIZE);
    cache = (u8_t *)   malloc (LOS_SPIFFS_CACHE_SIZE);

    if ((fs == NULL) || (wbuf == NULL) || (fds == NULL) || (cache == NULL)) {
        vlog_error("fail to malloc memory in SPIFFS, <malloc.c> is needed,"
                "make sure it is added");
        goto err_free;
    }
    memset(fs, 0, sizeof (spiffs));

    ret = SPIFFS_mount(fs, config, wbuf, fds, LOS_SPIFFS_FD_SIZE, cache,
                       LOS_SPIFFS_CACHE_SIZE, NULL);

    if (ret == SPIFFS_ERR_NOT_A_FS) {
        vlog_info ("formating fs...");

        SPIFFS_format(fs);

        ret = SPIFFS_mount(fs, config, wbuf, fds, LOS_SPIFFS_FD_SIZE, cache,
                           LOS_SPIFFS_CACHE_SIZE, NULL);
    }

    if (ret != SPIFFS_OK) {
        vlog_error("format fail!");
        goto err_unmount;
    }

    ret = vfs_mount("spiffs", path, fs);

    if (ret == 0) {
        s_cb.fs = fs;
        s_cb.wbuf = wbuf;
        s_cb.fds = fds;
        s_cb.cache = cache;
        return 0;
    }

    vlog_error("failed to mount!");

err_unmount:
    SPIFFS_unmount(fs);
err_free:
    SAFE_FREE(fs);
    SAFE_FREE(wbuf);
    SAFE_FREE(fds);
    SAFE_FREE(cache);
    return ret;
}

int spiffs_unmount(const char *path)
{
    int ret = -1;

    if (path == NULL || s_cb.fs == NULL) {
        return ret;
    }
    SPIFFS_unmount(s_cb.fs);
    ret = vfs_unmount(path);

    SAFE_FREE(s_cb.fs);
    SAFE_FREE(s_cb.wbuf);
    SAFE_FREE(s_cb.fds);
    SAFE_FREE(s_cb.cache);

    return ret;
}

