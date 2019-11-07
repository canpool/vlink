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

#ifndef __JFFS2_OS_H__
#define __JFFS2_OS_H__

#include <stdint.h>
#include "vos.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct completion {
    int reserve;
};

struct task_struct {
    int reserve;
};


#define cond_resched() do { } while(0)
#define signal_pending(x) (0)


/* semaphore / mutex */
typedef struct jffs2_mutex {
    vmutex_t mux;
}jffs2_mutex_t;

#define semaphore jffs2_mutex

#define DECLARE_MUTEX(x)            jffs2_mutex_t x = { V_MUTEX_INVALID };
#define DECLARE_MUTEX_LOCKED(x)     jffs2_mutex_t x = { V_MUTEX_INVALID };

#define INIT_MUTEX(x)               init_MUTEX(x)
#define DESTROY_MUTEX(x)            destroy_MUTEX(x)

#define init_MUTEX(sem)             jffs2_mutex_init((jffs2_mutex_t *)sem)
#define init_MUTEX_LOCKED(sem)      do { \
                                        if(0 == jffs2_mutex_init((jffs2_mutex_t *)sem)) \
                                            jffs2_mutex_lock((jffs2_mutex_t *)sem); \
                                    } while(0)
#define destroy_MUTEX(sem)          jffs2_mutex_release((jffs2_mutex_t *)sem)
#define down(sem)                   jffs2_mutex_lock((jffs2_mutex_t *)sem)
#define down_interruptible(sem)     (jffs2_mutex_lock((jffs2_mutex_t *)sem), 0)
#define down_trylock(sem)           jffs2_mutex_trylock((jffs2_mutex_t *)sem)
#define up(sem)                     jffs2_mutex_unlock((jffs2_mutex_t *)sem)

struct rw_semaphore {
    vsem_t sem;
};

#define down_read(sem)
#define down_read_trylock(sem)
#define down_write(sem)
#define down_write_trylock(sem)
#define up_read(sem)
#define up_write(sem)
#define downgrade_write(sem)

int jffs2_mutex_init(jffs2_mutex_t *mutex);
int jffs2_mutex_lock(jffs2_mutex_t *mutex);
int jffs2_mutex_trylock(jffs2_mutex_t *mutex);
int jffs2_mutex_unlock(jffs2_mutex_t *mutex);
int jffs2_mutex_release(jffs2_mutex_t *mutex);

/* Event */
typedef struct {
    vevent_t event;
} jffs2_flag_t;

#define JFFS2_FLAG_WAITMODE_AND (0) /* all bits must be set */
#define JFFS2_FLAG_WAITMODE_OR  (2) /* any bit must be set  */
#define JFFS2_FLAG_WAITMODE_CLR (1) /* clear when satisfied */

int      jffs2_flag_init(jffs2_flag_t *flag);
int      jffs2_flag_setbits(jffs2_flag_t *flag, uint32_t value);
int      jffs2_flag_maskbits(jffs2_flag_t *flag, uint32_t value);
uint32_t jffs2_flag_wait(jffs2_flag_t *flag, uint32_t pattern, uint32_t mode);
uint32_t jffs2_flag_timed_wait(jffs2_flag_t *flag, uint32_t pattern, uint32_t mode, uint32_t timeout);
int      jffs2_flag_destroy(jffs2_flag_t *flag);


/* thread */
typedef struct {
    vtask_t task;
} jffs2_thread_t;

typedef void (*jffs2_thread_fn)(void *arg);

int jffs2_thread_new(jffs2_thread_t *thread, char *name, jffs2_thread_fn function,
                            void *arg, int stacksize, int prio);
int jffs2_thread_delete(jffs2_thread_t *thread);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_OS_H__ */
