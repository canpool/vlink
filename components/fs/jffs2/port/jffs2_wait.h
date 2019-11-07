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

#ifndef __JFFS2_WAIT_H__
#define __JFFS2_WAIT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct { int none;} wait_queue_head_t;

#define init_waitqueue_head(wait) do{} while (0)
#define add_wait_queue(wait,new_wait) do{} while (0)
#define remove_wait_queue(wait,old_wait) do{} while (0)
#define DECLARE_WAITQUEUE(wait,current) do{} while (0)

static inline void wake_up(wait_queue_head_t *erase_wait)
{ /* Only used for waking up threads blocks on erases. Not used in eCos */ }



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_WAIT_H__ */
