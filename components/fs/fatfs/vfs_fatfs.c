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

#include "vfs_fatfs.h"
#include "vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct disk_mgt disk;

/**
  * @brief  Links a compatible diskio driver/lun id and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits.
  * @param  drv: pointer to the disk IO Driver structure
  * @param  lun : only used for USB Key Disk to add multi-lun management
            else the parameter must be equal to 0
  * @retval Returns -1 in case of failure, otherwise return the drive (0 to volumes).
  */
static int fatfs_link_driver(const struct diskio_drv *drv, uint8_t lun)
{
    int ret = -1;
    int i;

    if (disk.num >= FF_VOLUMES) {
        return ret;
    }
    for (i = 0; i < FF_VOLUMES; i++) {
        if(disk.dev[i].drv != 0) {
            continue;
        }
        disk.dev[disk.num].state = 0;
        disk.dev[disk.num].drv = drv;
        disk.dev[disk.num].lun = lun;
        disk.num++;
        return i;
    }
    return ret;
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  drive: the disk drive (0 to volumes)
  * @param  lun : not used
  * @retval Returns -1 in case of failure, otherwise return the drive (0 to volumes).
  */
static int fatfs_unlink_driver(uint8_t drive, uint8_t lun)
{
    int ret = -1;

    if (disk.num >= 1 && drive < FF_VOLUMES) {
        if(disk.dev[drive].drv != 0) {
            disk.dev[drive].state= 0;
            disk.dev[drive].drv = 0;
            disk.dev[drive].lun = 0;
            disk.num--;
            return drive;
        }
    }

    return ret;
}

int fatfs_register(const struct diskio_drv *drv)
{
    return fatfs_link_driver(drv, 0);
}

int fatfs_unregister(uint8_t drive)
{
    return fatfs_unlink_driver(drive, 0);
}

static int fatfs_flags_get(int oflags)
{
    int flags = 0;

    switch (oflags & O_ACCMODE) {
    case O_RDONLY:
        flags |= FA_READ;
        break;
    case O_WRONLY:
        flags |= FA_WRITE;
        break;
    case O_RDWR:
        flags |= FA_READ | FA_WRITE;
        break;
    default:
        break;
    }

    if (oflags & O_CREAT) {
        flags |= FA_OPEN_ALWAYS;
    }

    if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
        flags |= FA_CREATE_NEW;
    }

    if (oflags & O_TRUNC) {
        flags |= FA_CREATE_ALWAYS;
    }

    if (oflags & O_APPEND) {
        flags |= FA_READ | FA_WRITE | FA_OPEN_APPEND;
    }

    return flags;
}

static int fatfs_op_open(struct file *file, const char *path, int flags, int mode)
{
    FRESULT     res;
    FIL        *fp;
    FILINFO     info = {0};

    fp = (FIL *)malloc(sizeof(FIL));
    if (fp == NULL) {
        vlog_error("fail to malloc memory in FATFS, <malloc.c> is needed, make sure it is added");
        return -1;
    }

    if (!(flags & O_CREAT) && (flags & O_TRUNC)) {
        res = f_stat(path, &info);
        if(res != FR_OK) {
            vlog_error("f_stat() failed, res=%d", res);
            free(fp);
            return -1;
        }
    }

    res = f_open(fp, path, fatfs_flags_get(flags));
    if (res != FR_OK) {
        vlog_error("f_open() failed, res=%d", res);
        free(fp);
        return -1;
    }
    file->f_data = (void *)fp;

    return 0;
}

static int fatfs_op_close(struct file *file)
{
    FRESULT     res;
    FIL        *fp = (FIL *)file->f_data;

    if (fp == NULL) {
        return -1;
    }

    res = f_close(fp);
    if (res != FR_OK) {
        vlog_error("f_close() failed, res=%d", res);
        return -1;
    }
    free(fp);
    file->f_data = NULL;

    return 0;
}

static ssize_t fatfs_op_read(struct file *file, char *buff, size_t bytes)
{
    ssize_t     size = -1;
    FRESULT     res;
    FIL        *fp = (FIL *)file->f_data;

    if (fp == NULL) {
        return -1;
    }

    res = f_read(fp, buff, bytes, (UINT *)&size);
    if (res != FR_OK) {
        vlog_error("f_read() failed, res=%d", res);
        return -1;
    }
    return size;
}

static ssize_t fatfs_op_write(struct file *file, const char *buff, size_t bytes)
{
    ssize_t  size = -1;
    FRESULT  res;
    FIL     *fp = (FIL *)file->f_data;

    if (fp == NULL) {
        return -1;
    }

    res = f_write(fp, buff, bytes, (UINT *)&size);
    if (res != FR_OK || size == 0){
        vlog_error("f_write() failed, res=%d", res);
        return -1;
    }
    return size;
}

static off_t fatfs_op_lseek(struct file *file, off_t off, int whence)
{
    FRESULT     res;
    FIL        *fp = (FIL *)file->f_data;

    if (fp == NULL) {
        return -1;
    }

    switch (whence) {
    case 0: // SEEK_SET
        break;
    case 1: // SEEK_CUR
        off += f_tell(fp);
        break;
    case 2: // SEEK_END
        off += f_size(fp);
        break;
    default:
        return -1;
    }
    if (off < 0) {
        vlog_error("off is %ld now, whence is %d", off, whence);
        return -1;
    }

    res = f_lseek(fp, off);
    if (res != FR_OK) {
        vlog_error("f_lseek() failed, res=%d", res);
    	return -1;
    }
    return off;
}

static int fatfs_op_stat(struct mnt *mp, const char *path, struct stat *stat)
{
    FRESULT res;
    FILINFO info = {0};

    res = f_stat(path, &info);
    if (res != FR_OK) {
        vlog_error("f_stat() failed, res=%d", res);
        return -1;
    }

    stat->st_size = info.fsize;
    if (info.fattrib & AM_DIR) {
        stat->st_mode = S_IFDIR;
    } else {
        stat->st_mode = S_IFREG;
    }

    return 0;
}

static int fatfs_op_unlink(struct mnt *mp, const char *path)
{
    FRESULT res = f_unlink(path);
    if (res != FR_OK) {
        vlog_error("f_unlink() failed, res=%d", res);
        return -1;
    }
    return 0;
}

static int fatfs_op_rename(struct mnt *mp, const char *path_old,
                             const char *path_new)
{
    FRESULT res = f_rename(path_old, path_new);
    if (res != FR_OK) {
        vlog_error("f_rename() failed, res=%d", res);
        return -1;
    }
    return 0;
}

static int fatfs_op_sync(struct file *file)
{
    FRESULT     res;
    FIL        *fp = (FIL *)file->f_data;

    if (fp == NULL) {
        return -1;
    }

    res = f_sync(fp);
    if (res != FR_OK) {
        vlog_error("f_sync() failed, res=%d", res);
        return -1;
    }
    return 0;
}

static int fatfs_op_opendir(struct dir *dir, const char *path)
{
    FRESULT  res;
    DIR     *dp;

    dp = (DIR *)malloc(sizeof(DIR));
    if (dp == NULL) {
        vlog_error("fail to malloc memory in FATFS, <malloc.c> is needed,"
                   "make sure it is added");
        return -1;
    }

    res = f_opendir(dp, path);
    if (res != FR_OK) {
        vlog_error("f_opendir() failed, res=%d", res);
        free(dp);
        return -1;
    }

    dir->d_data = (void *)dp;

    return 0;
}

static int fatfs_op_readdir(struct dir *dir, struct dirent *dent)
{
    FRESULT     res;
    DIR        *dp = (DIR *)dir->d_data;
    FILINFO     info;
    int         len;

    if (dp == NULL) {
        return -1;
    }

    res = f_readdir(dp, &info);
    if (res != FR_OK) {
        vlog_error("f_readdir() failed, res=%d", res);
        return -1;
    }

    len = min(sizeof(info.fname), sizeof(dent->d_name)) - 1;
    strncpy((char *)dent->d_name, (const char *)info.fname, len);
    dent->d_name[len] = '\0';
    dent->d_size = info.fsize;

    if (info.fattrib == AM_DIR) {
        dent->d_type = DT_DIR;
    } else {
        dent->d_type = DT_REG;
    }

    return 0;
}

static int fatfs_op_closedir(struct dir *dir)
{
    FRESULT  res;
    DIR     *dp = (DIR *)dir->d_data;

    if (dp == NULL) {
        return -1;
    }

    res = f_closedir(dp);
    if (res != FR_OK) {
        vlog_error("f_closedir() failed, res=%d", res);
        return -1;
    }
    free(dp);
    dir->d_data = NULL;

    return 0;
}

static int fatfs_op_mkdir(struct mnt *mp, const char *path, int mode)
{
    FRESULT res = f_mkdir(path);
    if (res != FR_OK) {
        vlog_error("f_mkdir() failed, res=%d", res);
        return -1;
    }
    return 0;
}

static int fatfs_op_rmdir(struct mnt *mp, const char *path)
{
    FRESULT res = f_rmdir(path);
    if (res != FR_OK) {
        vlog_error("f_rmdir() failed, res=%d", res);
        return -1;
    }
    return 0;
}


static struct file_ops fatfs_ops =
{
    fatfs_op_open,
    fatfs_op_close,
    fatfs_op_read,
    fatfs_op_write,
    fatfs_op_lseek,
    fatfs_op_stat,
    fatfs_op_unlink,
    fatfs_op_rename,
    NULL,               /* ioctl not supported for now */
    fatfs_op_sync,
    fatfs_op_opendir,
    fatfs_op_readdir,
    fatfs_op_closedir,
    fatfs_op_mkdir,
    fatfs_op_rmdir
};

static struct file_system fatfs_fs =
{
    "fatfs",
    &fatfs_ops,
    NULL,
    0
};

int fatfs_init(void)
{
    static int fatfs_inited = 0;

    if (fatfs_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&fatfs_fs) != 0) {
        vlog_info("failed to register fatfs!");
        return -1;
    }

    fatfs_inited = 1;

    vlog_info("register fatfs done!");

    return 0;
}

static FATFS *s_fatfs = NULL;

int fatfs_mount(const char *path, struct diskio_drv *drv, uint8_t *drive)
{
    int     s_drive;
    char    dpath[10] = {0};
    int     ret = -1;
    BYTE   *work_buff = NULL;
    FRESULT res;
    FATFS  *fs = NULL;

    s_drive = fatfs_register(drv);
    if (s_drive < 0) {
        vlog_error("failed to register diskio!");
        return -1;
    }
    fs = (FATFS *)malloc(sizeof(FATFS));
    if (fs == NULL) {
        vlog_error("fail to malloc memory in FATFS, <malloc.c> is needed,"
                   "make sure it is added");
        goto err;
    }
    memset(fs, 0, sizeof(FATFS));
    sprintf(dpath, "%d:/", s_drive);
    res = f_mount(fs, (const TCHAR *)dpath, 1);
    if (res == FR_NO_FILESYSTEM) {
        work_buff = (BYTE *)malloc(FF_MAX_SS);
        if (work_buff == NULL) {
            goto err_free;
        }
        memset(work_buff, 0, FF_MAX_SS);
        res = f_mkfs((const TCHAR *)dpath, FM_ANY, 0, work_buff, FF_MAX_SS);
        if (res == FR_OK) {
            res = f_mount(NULL, (const TCHAR *)dpath, 1);
            res = f_mount(fs, (const TCHAR *)dpath, 1);
        }
        free(work_buff);
    }
    if(res != FR_OK) {
        vlog_error("failed to mount fatfs, res=%d!", res);
        goto err_free;
    }

    ret = vfs_mount("fatfs", path, fs);
    if (ret == 0) {
        *drive = s_drive;
        s_fatfs = fs;
        return 0;
    }

    vlog_error("failed to mount!");

err_free:
    if (fs != NULL) {
        free(fs);
    }
err:
    fatfs_unregister(s_drive);
    return ret;
}

int fatfs_unmount(const char *path, uint8_t drive)
{
    char dpath[10] = {0};

    sprintf(dpath, "%d:/", drive);
    fatfs_unregister(drive);
    f_mount(NULL, (const TCHAR *)dpath, 1);
    vfs_unmount(path);

    if (s_fatfs) {
        free(s_fatfs);
        s_fatfs = NULL;
    }

    return 0;
}

