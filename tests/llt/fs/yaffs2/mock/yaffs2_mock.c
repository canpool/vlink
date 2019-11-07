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

#include "vfs_yaffs.h"
#include "vlog.h"

#include <stdio.h>
#include <string.h>

#include "yportenv.h"
#include "yaffs_trace.h"

#include "mock_nand_flash.h"


static struct yaffs_dev *dev = NULL;
static nand_dev_t nand_dev;

unsigned int yaffs_trace_mask = YAFFS_TRACE_BAD_BLOCKS | YAFFS_TRACE_ALWAYS | 0;
//unsigned int yaffs_wr_attempts = YAFFS_WR_ATTEMPTS;
//unsigned int yaffs_auto_checkpoint = 1;
//unsigned int yaffs_gc_control = 1;
//unsigned int yaffs_bg_enable = 1;
//unsigned int yaffs_auto_select = 1;

static int nand_write_chunk(struct yaffs_dev *dev, int nand_chunk,
				   const u8 *data, int data_len,
				   const u8 *oob, int oob_len)
{
    int ret;

    nand_addr_t addr;
    uint32_t chunks_per_block = dev->param.chunks_per_block;
    uint32_t page_num = data_len / dev->param.total_bytes_per_chunk;
    uint32_t spare_num = oob_len / dev->param.spare_bytes_per_chunk;

    addr.page = nand_chunk % chunks_per_block;
    addr.block = nand_chunk / chunks_per_block;
    addr.zone = addr.block / nand_dev.config.zone_size;
    addr.block %= nand_dev.config.zone_size;

    if (data && page_num > 0)
    {
        ret = hal_nand_write_page(&nand_dev, &addr, (uint8_t *)data, page_num);
        if (ret != 0)
        {
            return YAFFS_FAIL;
        }
    }
    if (oob && oob_len > 0)
    {
        ret = hal_nand_write_spare(&nand_dev, &addr, (uint8_t *)oob, oob_len);
        if (ret != 0)
        {
            return YAFFS_FAIL;
        }
    }

    return YAFFS_OK;
}

static int nand_read_chunk(struct yaffs_dev *dev, int nand_chunk,
				   u8 *data, int data_len,
				   u8 *oob, int oob_len,
				   enum yaffs_ecc_result *ecc_result)
{
    int ret;

    nand_addr_t addr;
    uint32_t chunks_per_block = dev->param.chunks_per_block;
    uint32_t page_num = data_len / dev->param.total_bytes_per_chunk;
    uint32_t spare_num = oob_len / dev->param.spare_bytes_per_chunk;

    addr.page = nand_chunk % chunks_per_block;
    addr.block = nand_chunk / chunks_per_block;
    addr.zone = addr.block / nand_dev.config.zone_size;
    addr.block %= nand_dev.config.zone_size;

    if (data && page_num > 0)
    {
        ret = hal_nand_read_page(&nand_dev, &addr, (uint8_t *)data, page_num);
        if (ret != 0)
        {
            return YAFFS_FAIL;
        }
    }
    if (oob && oob_len > 0)
    {
        ret = hal_nand_read_spare(&nand_dev, &addr, (uint8_t *)oob, oob_len);
        if (ret != 0)
        {
            return YAFFS_FAIL;
        }
    }
    if (ecc_result)
    {
        *ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
    }

    return YAFFS_OK;
}

static int nand_erase_block(struct yaffs_dev *dev, int block_no)
{
    int ret;
    nand_addr_t addr;

    addr.page = 0;
    addr.zone = 0;
    addr.block = block_no;

    ret = hal_nand_erase_block(&nand_dev, &addr);
    if (ret != 0) {
        return YAFFS_FAIL;
    }
    return YAFFS_OK;
}

static int nand_mark_bad(struct yaffs_dev *dev, int block_no)
{
    /* add code according to your nand driver */
    return YAFFS_OK;
}

static int nand_check_bad(struct yaffs_dev *dev, int block_no)
{
    /* add code according to your nand driver */
    return YAFFS_OK;
}

static int nand_init(struct yaffs_dev *dev)
{
    return YAFFS_OK;
}

static struct yaffs_dev *yaffs_install_drv(const char *name)
{
    struct yaffs_dev *dev = NULL;
    struct yaffs_param *param;
    struct yaffs_driver *drv;

    if (name == NULL) {
        return NULL;
    }

    dev = (struct yaffs_dev *)malloc(sizeof(struct yaffs_dev));
    if (dev == NULL) {
        return NULL;
    }
    memset(dev, 0, sizeof(struct yaffs_dev));

    drv = &dev->drv;

    drv->drv_write_chunk_fn = nand_write_chunk;
    drv->drv_read_chunk_fn = nand_read_chunk;
    drv->drv_erase_fn = nand_erase_block;
    drv->drv_mark_bad_fn = nand_mark_bad;
    drv->drv_check_bad_fn = nand_check_bad;
    drv->drv_initialise_fn = nand_init;

    param = &dev->param;

    param->name = name;
    param->total_bytes_per_chunk = nand_dev.config.page_size;
    param->chunks_per_block = nand_dev.config.block_size;
    param->spare_bytes_per_chunk = nand_dev.config.spare_area_size;
    param->start_block = 0;
    param->end_block = nand_dev.config.zone_number * nand_dev.config.zone_size - 1;
    param->use_nand_ecc = 1;

    param->n_reserved_blocks = 5;
    param->wide_tnodes_disabled = 0;
    param->refresh_period = 1000;
    param->n_caches = 0; // Use caches

    param->enable_xattr = 1;

    yaffs_add_device(dev);

    return dev;
}

int yaffs_start_up(void)
{
	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_OSInitialisation();

	return 0;
}

int mock_yaffs2_init(void)
{
    int ret;
    unsigned int yaffs_trace_mask =
//                        YAFFS_TRACE_SCAN |
//                        YAFFS_TRACE_GC |
//                        YAFFS_TRACE_ERASE |
//                        YAFFS_TRACE_ERROR |
//                        YAFFS_TRACE_TRACING |
//                        YAFFS_TRACE_ALLOCATE |
//                        YAFFS_TRACE_BAD_BLOCKS |
//                        YAFFS_TRACE_CHECKPOINT |
//                        YAFFS_TRACE_VERIFY |
//                        YAFFS_TRACE_ALWAYS |
//                        YAFFS_TRACE_MOUNT |
                        0;

    yaffs_set_trace(yaffs_trace_mask);

    hal_nand_init(&nand_dev);

    dev = yaffs_install_drv(""); // the name is equal to yaffs_mount path
    if (dev == NULL) return -1;

    (void)yaffs_start_up();
    (void)yaffs2_init();

    ret = yaffs2_mount("/yaffs2/");

    if (ret < 0) {
        vlog_error("failed to mount yaffs2!");
        free(dev);
        dev = NULL;
    }

    return ret;
}

int mock_yaffs2_exit(void)
{
    hal_nand_finalize(&nand_dev);
    int ret = yaffs2_unmount("/yaffs2/");
    if (dev) {
        yaffs_remove_device(dev);
        free(dev);
        dev = NULL;
    }
    return ret;
}

