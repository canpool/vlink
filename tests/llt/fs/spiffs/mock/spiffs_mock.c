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

#include <stdio.h>
#include <string.h>

#include "mock_spi_flash.h"

#define SPIFFS_PHYS_SIZE    1024 * 1024
#define PHYS_ERASE_SIZE     64 * 1024
#define LOG_BLOCK_SIZE      64 * 1024
#define LOG_PAGE_SIZE       SPI_FLASH_PAGESIZE

static s32_t mock_spiffs_read(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *buff)
{
    (void)hal_spi_flash_read ((void *) buff, size, addr);

    return SPIFFS_OK;
}

static s32_t mock_spiffs_write(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *buff)
{
    (void)hal_spi_flash_write ((void *) buff, size, &addr);

    return SPIFFS_OK;
}

static s32_t mock_spiffs_erase(struct spiffs_t *fs, u32_t addr, u32_t size)
{
    (void)hal_spi_flash_erase (addr, size);

    return SPIFFS_OK;
}

static spiffs_config s_config =
{
    .hal_read_f         = mock_spiffs_read,
    .hal_write_f        = mock_spiffs_write,
    .hal_erase_f        = mock_spiffs_erase,
    .phys_addr          = 0,
    .phys_size          = SPIFFS_PHYS_SIZE,
    .phys_erase_block   = PHYS_ERASE_SIZE,
    .log_block_size     = LOG_BLOCK_SIZE,
    .log_page_size      = LOG_PAGE_SIZE,
    .fh_ix_offset       = TEST_SPIFFS_FILEHDL_OFFSET
};

int mock_spiffs_init(void)
{
    (void)spiffs_init();

    if (spiffs_mount("/spiffs/", &s_config) != 0) {
        printf("failed to mount spiffs!\n");
        return -1;
    }

    return 0;
}

int mock_spiffs_exit(void)
{
    return spiffs_unmount("/spiffs/");
}

