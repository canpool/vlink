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

#include "yaffs_osglue.h"
#include "vos.h"


static vmutex_t yaffs_mutex = V_MUTEX_INVALID;
static int yaffsfs_lastError;

void yaffsfs_Lock(void)
{
    (void)vos_mutex_lock(yaffs_mutex);
}

void yaffsfs_Unlock(void)
{
    (void)vos_mutex_unlock(yaffs_mutex);
}

u32 yaffsfs_CurrentTime(void)
{
    return (u32)vos_sys_time();
}

int yaffsfs_GetLastError(void)
{
    return yaffsfs_lastError;
}

void yaffsfs_SetError(int err)
{
    yaffsfs_lastError = err;
}

void *yaffsfs_malloc(size_t size)
{
    return vos_malloc(size);
}

void yaffsfs_free(void *ptr)
{
    vos_free(ptr);
}

int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
    /* add code according to the hardware configuration */
    return 0;
}

void yaffsfs_OSInitialisation(void)
{
    (void)vos_mutex_init(&yaffs_mutex);
}

void yaffs_bug_fn(const char *file_name, int line_no)
{
    printf("yaffs bug detected %s:%d\n", file_name, line_no);
}

