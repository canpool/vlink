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

#include "uffs/uffs_os.h"
#include "uffs/uffs_public.h"
#include "uffs_config.h"

#include <stdio.h>
#include <stdlib.h>

#include "vos.h"

#define PFX "os  : "
#define UFFS_TAG "uffs"

int uffs_SemCreate(OSSEM *sem)
{
    return vos_mutex_init((vmutex_t *)sem);
}

int uffs_SemWait(OSSEM sem)
{
    return vos_mutex_lock((vmutex_t)sem);
}

int uffs_SemSignal(OSSEM sem)
{
    return vos_mutex_unlock((vmutex_t)sem);
}

int uffs_SemDelete(OSSEM *sem)
{
    vos_mutex_destroy((vmutex_t *)sem);
    *sem = (OSSEM)V_SEM_INVALID;
    return 0;
}

int uffs_OSGetTaskId(void)
{
    return 0;
}

unsigned int uffs_GetCurDateTime(void)
{
    // FIXME: return system time, please modify this for your platform !
    //          or just return 0 if you don't care about file time.
    return (unsigned int)vos_sys_time();
}

#if CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR > 0
static void *sys_malloc(struct uffs_DeviceSt *dev, unsigned int size)
{
    uffs_Perror(UFFS_MSG_NORMAL, "system memory alloc %d bytes", size);
    return vos_malloc(size);
}

static URET sys_free(struct uffs_DeviceSt *dev, void *p)
{
    vos_free(p);
    return U_SUCC;
}

void uffs_MemSetupSystemAllocator(uffs_MemAllocator *allocator)
{
    allocator->malloc = sys_malloc;
    allocator->free = sys_free;
}
#endif

/* debug message output throught 'printf' */
static void output_dbg_msg(const char *msg);
static struct uffs_DebugMsgOutputSt m_dbg_ops = {
    output_dbg_msg,
    NULL,
};

static void output_dbg_msg(const char *msg)
{
    printf("%s", msg);
}

void uffs_SetupDebugOutput(void)
{
    uffs_InitDebugMessageOutput(&m_dbg_ops, UFFS_MSG_NOISY);
}
