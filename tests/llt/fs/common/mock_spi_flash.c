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

#include "mock_spi_flash.h"

#include <string.h>


#ifndef CHECK_RET_RETURN
#define CHECK_RET_RETURN(ret) \
    do \
    { \
        if ((ret) < 0) \
        { \
            return ret; \
        } \
    } while (0)
#endif

static char flash[SPI_FLASH_TOTAL_SIZE] = {0};

static int prv_spi_flash_write_page(const uint8_t* buf, uint32_t addr, int32_t len)
{
    memcpy(flash + addr, buf, len);
    return 0;
}

static int prv_spi_flash_erase_sector(uint32_t addr)
{
    memset(flash + addr, 0xff, SPI_FLASH_SECTOR);
    return 0;
}

int hal_spi_flash_erase(uint32_t addr, int32_t len)
{
    uint32_t begin;
    uint32_t end;
    int i;

    if (len < 0
        || addr > SPI_FLASH_TOTAL_SIZE
        || addr + len > SPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    begin = addr / SPI_FLASH_SECTOR * SPI_FLASH_SECTOR;
    end = (addr + len - 1) / SPI_FLASH_SECTOR * SPI_FLASH_SECTOR;

    for (i = begin; i <= end; i += SPI_FLASH_SECTOR)
    {
        if (prv_spi_flash_erase_sector(i) == -1)
        {
            return -1;
        }
    }

    return 0;
}

int hal_spi_flash_write(const void *buf, int32_t len, uint32_t *location)
{
    const uint8_t* pbuf = (const uint8_t*)buf;
    int page_cnt = 0;
    int remain_cnt = 0;
    int temp = 0;
    uint32_t loc_addr;
    uint8_t addr = 0;
    uint8_t count = 0;
    int i;
    int ret = 0;

    if (NULL == pbuf
        || NULL == location
        || len < 0
        || *location > SPI_FLASH_TOTAL_SIZE
        || len + *location > SPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    loc_addr = *location;
    addr = loc_addr % SPI_FLASH_PAGESIZE;
    count = SPI_FLASH_PAGESIZE - addr;
    page_cnt = len / SPI_FLASH_PAGESIZE;
    remain_cnt = len % SPI_FLASH_PAGESIZE;

    if (addr == 0) /* addr is aligned to SPI_FLASH_PAGESIZE */
    {
        if (page_cnt == 0) /* len < SPI_FLASH_PAGESIZE */
        {
            ret = prv_spi_flash_write_page(pbuf, loc_addr, len);
            CHECK_RET_RETURN(ret);
        }
        else /* len > SPI_FLASH_PAGESIZE */
        {
            for (i = 0; i < page_cnt; ++i)
            {
                ret = prv_spi_flash_write_page(pbuf + i * SPI_FLASH_PAGESIZE, loc_addr, SPI_FLASH_PAGESIZE);
                CHECK_RET_RETURN(ret);
                loc_addr += SPI_FLASH_PAGESIZE;
            }

            ret = prv_spi_flash_write_page(pbuf + page_cnt * SPI_FLASH_PAGESIZE, loc_addr, remain_cnt);
            CHECK_RET_RETURN(ret);
        }
    }
    else /* addr is not aligned to SPI_FLASH_PAGESIZE */
    {
        if (page_cnt == 0) /* len < SPI_FLASH_PAGESIZE */
        {
            if (remain_cnt > count) /* (len + loc_addr) > SPI_FLASH_PAGESIZE */
            {
                temp = remain_cnt - count;

                ret = prv_spi_flash_write_page(pbuf, loc_addr, count);
                CHECK_RET_RETURN(ret);

                ret = prv_spi_flash_write_page(pbuf + count, loc_addr + count, temp);
                CHECK_RET_RETURN(ret);
            }
            else
            {
                ret = prv_spi_flash_write_page(pbuf, loc_addr, len);
                CHECK_RET_RETURN(ret);
            }
        }
        else /* len > SPI_FLASH_PAGESIZE */
        {
            len -= count;
            page_cnt = len / SPI_FLASH_PAGESIZE;
            remain_cnt = len % SPI_FLASH_PAGESIZE;

            ret = prv_spi_flash_write_page(pbuf, loc_addr, count);
            CHECK_RET_RETURN(ret);
            loc_addr += count;

            for (i = 0; i < page_cnt; ++i)
            {
                ret = prv_spi_flash_write_page(pbuf + count + i * SPI_FLASH_PAGESIZE, loc_addr, SPI_FLASH_PAGESIZE);
                CHECK_RET_RETURN(ret);
                loc_addr += SPI_FLASH_PAGESIZE;
            }

            if (remain_cnt != 0)
            {
                ret = prv_spi_flash_write_page(pbuf + count + page_cnt * SPI_FLASH_PAGESIZE, loc_addr, remain_cnt);
                CHECK_RET_RETURN(ret);
            }
        }
    }

    *location += len;
    return ret;
}

int hal_spi_flash_erase_write(const void* buf, int32_t len, uint32_t location)
{
    int ret = 0;

    ret = hal_spi_flash_erase(location, len);
    CHECK_RET_RETURN(ret);
    ret = hal_spi_flash_write(buf, len, &location);

    return ret;
}

int hal_spi_flash_read(void* buf, int32_t len, uint32_t location)
{
    int ret = 0;
    uint8_t* pbuf = (uint8_t*)buf;

    if (NULL == pbuf
        || len < 0
        || location > SPI_FLASH_TOTAL_SIZE
        || len + location > SPI_FLASH_TOTAL_SIZE)
    {
        return -1;
    }

    memcpy(pbuf, flash + location, len);

    return ret;
}

