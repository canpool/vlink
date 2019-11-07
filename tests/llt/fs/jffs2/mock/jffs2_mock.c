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

#include "vfs_jffs2.h"

#include <stdio.h>
#include <string.h>

#include "mock_spi_flash.h"

static int mock_jffs2_read(struct mtd_info *mtd, loff_t from, size_t len,
                                        size_t *retlen, u_char *buf)
{
    int ret = hal_spi_flash_read((void *)buf, len, from);
    if (ret < 0) {
        *retlen = 0;
        return ret;
    }
    *retlen = len;
    return 0;
}

static int mock_jffs2_write(struct mtd_info *mtd, loff_t to, size_t len,
                                        size_t *retlen, const u_char *buf)
{
    int ret = hal_spi_flash_write((void *)buf, len, (uint32_t *)&to);
    if (ret < 0) {
        *retlen = 0;
        return ret;
    }
    *retlen = len;
    return 0;
}

static int mock_jffs2_erase(struct mtd_info *mtd, loff_t from, size_t len)
{
    return hal_spi_flash_erase(from, len);
}


static struct mtd_info mtd_spi_flash =
{
    .type = MTD_TYPE_SPI_FLASH,
    .size = SPI_FLASH_TOTAL_SIZE,
    .erasesize = SPI_FLASH_SECTOR,

    .read = mock_jffs2_read,
    .write = mock_jffs2_write,
    .erase = mock_jffs2_erase
};


int mock_jffs2_init(int need_erase)
{
    int ret;

    if (need_erase) {
        (void)hal_spi_flash_erase(0, SPI_FLASH_TOTAL_SIZE);
    }

    (void)jffs2_init();

    ret = jffs2_mount("/jffs2/", &mtd_spi_flash);
    if (ret < 0) {
        printf("failed to mount jffs2!\n");
        return -1;
    }

    return 0;
}

int mock_jffs2_exit(void)
{
    return jffs2_unmount("/jffs2/");
}

