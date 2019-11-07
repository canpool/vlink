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
#include "vfs_fatfs.h"

#include <string.h>

#define SRAM_DEVICE_SIZE    (16 << 20)  // 16MB
#define SRAM_DEVICE_ADDR    (sram)

#define BLOCK_SIZE          512

static char sram[SRAM_DEVICE_SIZE] = {0};

static volatile DSTATUS state = STA_NOINIT;


static DSTATUS mock_fatfs_status(BYTE lun)
{
    return state;
}

static DSTATUS mock_fatfs_initialize(BYTE lun)
{
    state = STA_NOINIT;

    memset(sram, SRAM_DEVICE_SIZE, 0);
    if (1/* init ok */) {
        state &= ~STA_NOINIT;
    }
    return state;
}

static DRESULT mock_fatfs_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
    uint32_t BufferSize = (BLOCK_SIZE * count);
    uint8_t *pSramAddress = (uint8_t *)(SRAM_DEVICE_ADDR + (sector * BLOCK_SIZE));

    for (; BufferSize != 0; BufferSize--) {
        *buff++ = *(uint8_t *)pSramAddress++;
    }

    return RES_OK;
}

static DRESULT mock_fatfs_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    uint32_t BufferSize = (BLOCK_SIZE * count) + count;
    uint8_t *pSramAddress = (uint8_t *)(SRAM_DEVICE_ADDR + (sector * BLOCK_SIZE));

    for (; BufferSize != 0; BufferSize--) {
        *(uint8_t *)pSramAddress++ = *buff++;
    }

    return RES_OK;
}

static DRESULT mock_fatfs_ioctl(BYTE lun, BYTE cmd, void *buff)
{
    DRESULT res = RES_OK;

    switch (cmd)
    {
    case CTRL_SYNC:
        break;
    case GET_SECTOR_COUNT:
        *(DWORD *)buff = SRAM_DEVICE_SIZE / BLOCK_SIZE;
        break;
    case GET_SECTOR_SIZE:
        *(WORD *)buff = BLOCK_SIZE;
        break;
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = BLOCK_SIZE;
        break;
    default:
        res = RES_PARERR;
        break;
    }
    return res;
}

static struct diskio_drv sram_drv =
{
    mock_fatfs_initialize,
    mock_fatfs_status,
    mock_fatfs_read,
    mock_fatfs_write,
    mock_fatfs_ioctl
};

int mock_fatfs_init(void)
{
    int8_t drive = -1;

    (void)fatfs_init();

    if (fatfs_mount("/fatfs/", &sram_drv, (uint8_t *)&drive) < 0) {
        vlog_error("failed to mount fatfs!\n");
    }

    return drive;
}

int mock_fatfs_exit(int drive)
{
    if (drive < 0) {
        return -1;
    }
    return fatfs_unmount("/fatfs/", drive);
}

DWORD get_fattime (void)
{
    return 0;
}

