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

#include "vfs_uffs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uffs_config.h"
#include "uffs/uffs_public.h"
#include "uffs/uffs_fs.h"
#include "uffs/uffs_utils.h"
#include "uffs/uffs_core.h"
#include "uffs/uffs_mtb.h"

#include "uffs_fileem.h"


#define PAGE_DATA_SIZE    512
#define PAGE_SPARE_SIZE   16
#define PAGES_PER_BLOCK   32
#define TOTAL_BLOCKS      128

#define PAGE_SIZE					(PAGE_DATA_SIZE + PAGE_SPARE_SIZE)
#define BLOCK_DATA_SIZE				(PAGES_PER_BLOCK * PAGE_DATA_SIZE)
#define TOTAL_DATA_SIZE				(TOTAL_BLOCKS * BLOCK_DATA_SIZE)
#define BLOCK_SIZE					(PAGES_PER_BLOCK * PAGE_SIZE)
#define TOTAL_SIZE					(BLOCK_SIZE * TOTAL_BLOCKS)

#define MAX_MOUNT_TABLES		10
#define MAX_MOUNT_POINT_NAME	32

static uffs_Device demo_device = {0};
static struct uffs_MountTableEntrySt demo_mount = {
	&demo_device,
	0,    /* start from block 0 */
	-1,   /* use whole chip */
	"",   /* mount point, should be equal to uffs_Mount path */
	NULL
};

/* static alloc the memory */
static int static_buffer_pool[UFFS_STATIC_BUFF_SIZE(PAGES_PER_BLOCK, PAGE_SIZE, TOTAL_BLOCKS) / sizeof(int)];


static void setup_storage(struct uffs_StorageAttrSt *attr)
{
	attr->total_blocks = TOTAL_BLOCKS;			/* total blocks */
	attr->page_data_size = PAGE_DATA_SIZE;		/* page data size */
	attr->spare_size = PAGE_SPARE_SIZE;		  	/* page spare size */
	attr->pages_per_block = PAGES_PER_BLOCK;	/* pages per block */
	attr->block_status_offs = 4;				/* block status offset is 5th byte in spare */
    attr->ecc_opt = UFFS_ECC_SOFT;              /* use UFFS software ecc */
    attr->layout_opt = UFFS_LAYOUT_UFFS;        /* let UFFS do the spare layout */
}

static void setup_device(uffs_Device *dev)
{
	// using file emulator device
	dev->Init = femu_InitDevice;
	dev->Release = femu_ReleaseDevice;
	dev->attr = femu_GetStorage();
}


int mock_uffs_init(void)
{
	struct uffs_MountTableEntrySt *mtbl = &demo_mount;

    uffs_init();

//    uffs_SetupDebugOutput();    // setup debug output as early as possible

	/* setup flash storage attributes */
	setup_storage(femu_GetStorage());

	/* setup memory allocator */
	uffs_MemSetupStaticAllocator(&mtbl->dev->mem, static_buffer_pool, sizeof(static_buffer_pool));

	/* setup device: init, release, attr */
	setup_device(mtbl->dev);

	/* register mount table */
	uffs_RegisterMountTable(mtbl);

	/* mount it */
	uffs_mount("/uffs/");

	return uffs_InitFileSystemObjects() == U_SUCC ? 0 : -1;
}

int mock_uffs_exit(void)
{
	uffs_unmount("/uffs/");

	return uffs_ReleaseFileSystemObjects();
}

