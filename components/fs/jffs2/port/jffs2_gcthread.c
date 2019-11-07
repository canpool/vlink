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
/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 */

#include <jffs2_kernel.h>
#include "jffs2_nodelist.h"
#include "jffs2_os.h"

#if defined(CONFIG_JFFS2_GC_THREAD)

#define GC_THREAD_FLAG_TRIG         0x01
#define GC_THREAD_FLAG_STOP         0x02
#define GC_THREAD_FLAG_HAS_EXIT     0x04

uint32_t jffs2_current_time(void)
{
    return 0;
}

static void jffs2_garbage_collect_thread(void *data);

void jffs2_garbage_collect_trigger(struct jffs2_sb_info *c)
{
    struct super_block *sb = OFNI_BS_2SFFJ(c);

    /* Wake up the thread */
    D1(printk("jffs2_garbage_collect_trigger\n"));

    jffs2_flag_setbits(&sb->s_gc_thread_flags, GC_THREAD_FLAG_TRIG);
}

void jffs2_start_garbage_collect_thread(struct jffs2_sb_info *c)
{
    struct super_block *sb = NULL;
    int result;

    if(c == NULL) return;
    sb = OFNI_BS_2SFFJ(c);

    jffs2_flag_init(&sb->s_gc_thread_flags);
    jffs2_mutex_init(&sb->s_lock);

    D1(printk("jffs2_start_garbage_collect_thread\n"));
    /* Start the thread. Doesn't matter if it fails -- it's only an
     * optimisation anyway */
    result =  jffs2_thread_new(&sb->s_gc_thread,
                             "jffs2_gc_thread",
                             (jffs2_thread_fn)jffs2_garbage_collect_thread,
                             (void *)c,
                             CONFIG_JFFS2_GC_THREAD_STACK_SIZE,
                             CONFIG_JFFS2_GC_THREAD_PRIORITY
                            );
    if (result != 0)
    {
        /* how to deal with the following filed? */
        jffs2_mutex_release(&sb->s_lock);
        jffs2_flag_destroy(&sb->s_gc_thread_flags);
    }
}

void jffs2_stop_garbage_collect_thread(struct jffs2_sb_info *c)
{
    struct super_block *sb = OFNI_BS_2SFFJ(c);

    D1(printk("jffs2_stop_garbage_collect_thread\n"));
    /* Stop the thread and wait for it if necessary */

    jffs2_flag_setbits(&sb->s_gc_thread_flags, GC_THREAD_FLAG_STOP);

    D1(printk("jffs2_stop_garbage_collect_thread wait\n"));

    jffs2_flag_wait(&sb->s_gc_thread_flags,
                  GC_THREAD_FLAG_HAS_EXIT,
                  JFFS2_FLAG_WAITMODE_OR | JFFS2_FLAG_WAITMODE_CLR);

    // Kill and free the resources ...  this is safe due to the flag
    // from the thread.
    jffs2_thread_delete(&sb->s_gc_thread);
    jffs2_mutex_release(&sb->s_lock);
    jffs2_flag_destroy(&sb->s_gc_thread_flags);
}


static void jffs2_garbage_collect_thread(void *data)
{
    struct jffs2_sb_info *c = (struct jffs2_sb_info *)data;
    struct super_block *sb;
    uint32_t flag;

    if(c == NULL) return;
    sb = OFNI_BS_2SFFJ(c);

    D1(printk("jffs2_garbage_collect_thread START\n"));

    while(1)
    {
        flag = jffs2_flag_timed_wait(&sb->s_gc_thread_flags,
                      GC_THREAD_FLAG_TRIG | GC_THREAD_FLAG_STOP,
                      JFFS2_FLAG_WAITMODE_OR | JFFS2_FLAG_WAITMODE_CLR,
                      jffs2_current_time() + CONFIG_JFFS2_GS_THREAD_TICKS);

        if (flag & GC_THREAD_FLAG_STOP)
            break;

        D1(printk("jffs2: GC THREAD GC BEGIN\n"));

        // syscmode todo start
        if (jffs2_garbage_collect_pass(c) == -ENOSPC)
        {
            printf("No space for garbage collection. "
                   "Aborting JFFS2 GC thread\n");
            break;
        }
        // syscmode todo end
        D1(printk("jffs2: GC THREAD GC END\n"));
    }

    D1(printk("jffs2_garbage_collect_thread EXIT\n"));

    jffs2_flag_setbits(&sb->s_gc_thread_flags, GC_THREAD_FLAG_HAS_EXIT);
}

#endif

