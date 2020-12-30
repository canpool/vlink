/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VRING_H__ /* ring buffer */
#define __VRING_H__

#include "vpool.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *      head      tail
 *      +---------+---------+
 * buf  |/////////|         |
 *      +---------+---------+
 *      `------- size ------`
 *
 * empty: head == tail
 * full : head == (tail + 1) % size
 *        or
 *        tail - head == size
 */
typedef struct ring {
    size_t          head;   /* read  pos */
    size_t          tail;   /* write pos */
    size_t          size;   /* must be power of 2 */
    unsigned char * buf;
} vring_t;

/* inlines */

/**
 * @brief empty the ring buffer
 *
 * @param ring [IN] the ring buf
 */
v_inline void vring_reset(vring_t *ring) {
    ring->tail = ring->head = 0;
}

/**
 * @brief get the number of bytes in the ring buffer
 *
 * @param ring [IN] the ring buf
 */
v_inline size_t vring_len(vring_t *ring) {
    return ring->tail - ring->head;
}

/**
 * @brief check if the ring is empty
 *
 * @param ring [IN] the ring buf
 */
v_inline bool vring_empty(vring_t *ring) {
    return ring->tail == ring->head;
}

/**
 * @brief check if the ring is full
 *
 * @param ring [IN] the ring buf
 */
v_inline bool vring_full(vring_t *ring) {
    return vring_len(ring) == ring->size;
}

/**
 * @brief just return value from buf [head ~ tail] by index, data will not out of ring
 *
 * @param ring [IN] the ring buf
 * @param idx  [IN] the index from head to read
 */
v_inline unsigned char vring_peek(vring_t *ring, size_t idx) {
    return ring->buf[(ring->head + idx) & (ring->size - 1)];
}

/* externs */

int      vring_init(vring_t *ring, unsigned char *buf, size_t size);
vring_t *vring_create(size_t size);
void     vring_delete(vring_t *ring);
size_t   vring_put(vring_t *ring, unsigned char *buf, size_t len);
size_t   vring_put_force(vring_t *ring, unsigned char *buf, size_t len);
size_t   vring_putc(vring_t *ring, unsigned char byte);
size_t   vring_putc_force(vring_t *ring, unsigned char byte);
size_t   vring_get(vring_t *ring, unsigned char *buf, size_t len);
size_t   vring_getc(vring_t *ring, unsigned char *byte);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VRING_H__ */