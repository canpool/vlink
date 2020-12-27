/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __SOFTWARE_QUEUE_H__
#define __SOFTWARE_QUEUE_H__

#include "vos.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */

#define SWQUE_OP_RD     0
#define SWQUE_OP_WR     1

/* typedef */

typedef struct swque {
    const char    * name;
    size_t          msg_size;
    size_t          max_msgs;
    char          * msgs;
    size_t          idx[2];     /* 0: head/read, 1: tail/write */
    size_t          msg_cnt;    /* cached count */
    int             sync_mode;  /* 1 - use sem and mutex (can't be used in interrupt) */
    vsem_t          sem[2];     /* 0: read, 1: write */
    vmutex_t        lock;
} swque_t;

/* externs */

swque_t *swque_create(const char *name, size_t msg_size, size_t max_msgs, int sync_mode);
int swque_delete(swque_t *queue);
int swque_push(swque_t *queue, const void *buf, size_t size, uint32_t timeout);
int swque_pop(swque_t *queue, void *buf, size_t size, uint32_t timeout);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOFTWARE_QUEUE_H__ */