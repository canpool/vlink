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

#ifndef __JFFS2_CONFIG_H__
#define __JFFS2_CONFIG_H__


#define CONFIG_JFFS2_OS      /* unuse the default linux, use the specified OS */
#define CONFIG_JFFS2_DIRECT  /* unuse mtd */

#define FILE_PATH_MAX                128  /* the longest file path */
#define CONFIG_JFFS2_ENTRY_NAME_MAX  23
#define	JFFS2_NAME_MAX   CONFIG_JFFS2_ENTRY_NAME_MAX
#define	JFFS2_PATH_MAX   FILE_PATH_MAX

#define DEVICE_PART_MAX   1  /* the max partions on a nand deivce*/

/* memory page size in kernel/asm/page.h, it is correspond with flash read/write
 * option, so this size has a great impact on reading/writing speed */
#define CONFIG_JFFS2_PAGE_SHIFT  12  /* (1<<12) 4096bytes*/

/* jffs2 support relative dir, command "ls" will get
 * +-------------------------------+
 * |   finsh>>ls("/")              |
 * |   Directory /:                |
 * |   .                   <DIR>   |
 * |   ..                  <DIR>   |
 * |   dir1                <DIR>   |
 * +-------------------------------+
 */
#define CONFIG_JFFS2_NO_RELATIVEDIR

//#define CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE
#if defined(CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE)
	#define CYGPKG_FILEIO_DIRENT_DTYPE
#endif

#define CONFIG_JFFS2_WRITABLE   /* if not defined, jffs2 is read only*/

/* jffs2 debug output opion */
#define CONFIG_JFFS2_FS_DEBUG 		0  /* 1 or 2 */

/* jffs2 gc thread section */
#define CONFIG_JFFS2_GC_THREAD
#ifndef CONFIG_JFFS2_GC_THREAD_PRIORITY
#define CONFIG_JFFS2_GC_THREAD_PRIORITY     10 /* GC thread's priority */
#endif
#define CONFIG_JFFS2_GS_THREAD_TICKS        20  /* event timeout ticks */
#define CONFIG_JFFS2_GC_THREAD_TICKS        20  /* GC thread's running ticks */
#define CONFIG_JFFS2_GC_THREAD_STACK_SIZE   (1024*4)

//#define CONFIG_JFFS2_FS_WRITEBUFFER /* should not be enabled */

/* zlib section*/
//#define CONFIG_JFFS2_PROC
//#define CONFIG_JFFS2_ZLIB
//#define CONFIG_JFFS2_RTIME
//#define CONFIG_JFFS2_RUBIN
//#define CONFIG_JFFS2_CMODE_NONE
//#define CONFIG_JFFS2_CMODE_SIZE


#define GFP_KERNEL              0


#endif /* __JFFS2_CONFIG_H__ */
