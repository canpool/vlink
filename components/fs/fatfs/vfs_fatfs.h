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

#ifndef __VFS_FATFS_H__
#define __VFS_FATFS_H__

#include "diskio.h"
#include "ff.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DISK_STATE_INITIALIZED 1

struct diskio_drv {
    DSTATUS (*initialize)   (BYTE);                             /*!< Initialize Disk Drive  */
    DSTATUS (*status)       (BYTE);                             /*!< Get Disk Status        */
    DRESULT (*read)         (BYTE, BYTE *, DWORD, UINT);        /*!< Read Sector(s)         */
    DRESULT (*write)        (BYTE, const BYTE *, DWORD, UINT);  /*!< Write Sector(s)        */
    DRESULT (*ioctl)        (BYTE, BYTE, void *);               /*!< I/O control operation  */
};

struct disk_dev {
    uint8_t state;
    const struct diskio_drv *drv;
    uint8_t lun;
};

/* disk management */
struct disk_mgt {
    struct disk_dev dev[FF_VOLUMES];
    volatile uint8_t num;
};

int fatfs_init(void);
int fatfs_mount(const char *path, struct diskio_drv *drv, uint8_t *drive);
int fatfs_unmount(const char *path, uint8_t drive);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VFS_FATFS_H__ */
