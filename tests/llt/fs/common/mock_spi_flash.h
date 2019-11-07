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

#ifndef __MOCK_SPI_FLASH_H__
#define __MOCK_SPI_FLASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPI_FLASH_PAGESIZE          256
#define SPI_FLASH_SECTOR            4096
#define SPI_FLASH_TOTAL_SIZE        0x400000 // 16MB

int hal_spi_flash_erase(uint32_t addr, int32_t len);
int hal_spi_flash_write(const void *buf, int32_t len, uint32_t *location);
int hal_spi_flash_read(void* buf, int32_t len, uint32_t location);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __MOCK_SPI_FLASH_H__ */

