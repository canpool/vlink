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

#ifndef __JFFS2_PAGE_H__
#define __JFFS2_PAGE_H__

#include "jffs2_config.h"
#include "jffs2_debug.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define PAGE_SHIFT  CONFIG_JFFS2_PAGE_SHIFT
/* These aren't used by much yet. If that changes, you might want
   to make them actually correct :) */
#define PAGE_SIZE  (0x1 << PAGE_SHIFT)

#define PAGE_CACHE_SHIFT        PAGE_SHIFT
#define PAGE_CACHE_SIZE         PAGE_SIZE

#define PageLocked(pg) 1
#define Page_Uptodate(pg) 0
#define UnlockPage(pg)
#define PAGE_BUG(pg) BUG()
#define ClearPageUptodate(pg)
#define SetPageError(pg)
#define ClearPageError(pg)
#define SetPageUptodate(pg)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_PAGE_H__ */

