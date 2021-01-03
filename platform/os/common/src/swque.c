/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "swque.h"
#include "vos.h"

#include <string.h>

swque_t *swque_create(const char *name, size_t msg_size, size_t max_msgs, int sync_mode)
{
    swque_t *queue;

    queue = (swque_t *)vmem_malloc(sizeof(swque_t) + msg_size * max_msgs);
    if (queue == NULL) {
        return NULL;
    }
    memset(queue, 0, sizeof(swque_t));
    queue->name      = name;
    queue->msg_size  = msg_size;
    queue->max_msgs  = max_msgs;
    queue->sync_mode = sync_mode;
    queue->msgs      = (char *)((char *)queue + sizeof(swque_t));

    if (sync_mode) {
        if (vsem_init(&queue->sem[SWQUE_OP_RD], max_msgs, 0) != 0) {
            goto EXIT_FREE_QUEUE;
        }
        if (vsem_init(&queue->sem[SWQUE_OP_WR], max_msgs, max_msgs) != 0) {
            goto EXIT_DESTROY_RD_SEM;
        }
        if (vmutex_init(&queue->lock) != 0) {
            goto EXIT_DESTROY_WR_SEM;
        }
    }
    return queue;

EXIT_DESTROY_WR_SEM:
    vsem_destroy(&queue->sem[SWQUE_OP_WR]);
EXIT_DESTROY_RD_SEM:
    vsem_destroy(&queue->sem[SWQUE_OP_RD]);
EXIT_FREE_QUEUE:
    vmem_free(queue);
    return NULL;
}

int swque_delete(swque_t *queue)
{
    if (queue == NULL) {
        return VEINVAL;
    }
    if (queue->sync_mode) {
        vsem_destroy(&queue->sem[SWQUE_OP_RD]);
        vsem_destroy(&queue->sem[SWQUE_OP_WR]);
        vmutex_destroy(&queue->lock);
    }
    vmem_free(queue);
    return VOK;
}

static int __swque_push(swque_t *queue, const void *buf, size_t size)
{
    char *msg;

    if (queue->msg_cnt == queue->max_msgs) {
        return VEFULL;
    }
    msg = queue->msgs + queue->msg_size * queue->idx[SWQUE_OP_WR];
    memcpy(msg, buf, min(size, queue->msg_size));

    queue->idx[SWQUE_OP_WR] = (queue->idx[SWQUE_OP_WR] + 1) % queue->max_msgs;
    queue->msg_cnt++;

    return VOK;
}

int swque_push(swque_t *queue, const void *buf, size_t size, uint32_t timeout)
{
    int ret = VERR;

    if (queue == NULL || buf == NULL || size == 0) {
        return VEINVAL;
    }
    if (queue->sync_mode) {
        ret = VEFULL;
        if (vsem_timedwait(&queue->sem[SWQUE_OP_WR], timeout) == 0) {
            vmutex_lock(&queue->lock);
            ret = __swque_push(queue, buf, size);
            vmutex_unlock(&queue->lock);
            if (ret == 0) {
                vsem_post(&queue->sem[SWQUE_OP_RD]);
            }
        }
    } else {
        ret = __swque_push(queue, buf, size);
    }
    return ret;
}

static int __swque_pop(swque_t *queue, void *buf, size_t size)
{
    char *msg;

    if (queue->msg_cnt == 0) {
        return VEEMPTY;
    }
    msg = queue->msgs + queue->msg_size * queue->idx[SWQUE_OP_RD];
    memcpy(buf, msg, min(size, queue->msg_size));

    queue->idx[SWQUE_OP_RD] = (queue->idx[SWQUE_OP_RD] + 1) % queue->max_msgs;
    queue->msg_cnt--;

    return VOK;
}

int swque_pop(swque_t *queue, void *buf, size_t size, uint32_t timeout)
{
    int ret = VERR;

    if (queue == NULL || buf == NULL || size == 0) {
        return VEINVAL;
    }
    if (queue->sync_mode) {
        ret = VEEMPTY;
        if (vsem_timedwait(&queue->sem[SWQUE_OP_RD], timeout) == 0) {
            vmutex_lock(&queue->lock);
            ret = __swque_pop(queue, buf, size);
            vmutex_unlock(&queue->lock);
            if (ret == 0) {
                vsem_post(&queue->sem[SWQUE_OP_WR]);
            }
        }
    } else {
        ret = __swque_pop(queue, buf, size);
    }
    return ret;
}