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

#ifndef __JFFS2_SPINLOCK_H__
#define __JFFS2_SPINLOCK_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct { int none;} spinlock_t;
#define SPIN_LOCK_UNLOCKED (spinlock_t) { 0 }
#define DEFINE_SPINLOCK(x) spinlock_t x = {0};

#define spin_lock_init(lock)
#define spin_lock(lock)
#define spin_unlock(lock)
#define spin_lock_bh(lock)
#define spin_unlock_bh(lock)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_SPINLOCK_H__ */
