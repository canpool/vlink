/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vring.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief initialize a predefined ring using preallocated buffer
 *
 * @param ring  [IN] the predefined ring to be used
 * @param buf   [IN] the preallocated buffer to be used
 * @param size  [IN] the size of the preallocated buffer, must be power of 2
 *
 * @return 0 on success, negtive value on error
 */
int vring_init(vring_t *ring, unsigned char *buf, size_t size)
{
    if (ring == NULL || buf == NULL) {
        return -1;
    }

    if (size & (size - 1) || size == 0) {
        return -1;
    }

    ring->buf = buf;
    ring->size = size;
    ring->head = 0;
    ring->tail = 0;

    return 0;
}

/**
 * @brief create ring with a specificed size
 *
 * @param size [IN] the size of the buffer to be allocated, must be power of 2
 *
 * @return a pointer to the ring allocated, NULL on error
 */
vring_t *vring_create(size_t size)
{
    vring_t *ring;

    if (size & (size - 1) || size == 0) {
        return NULL;
    }

    ring = (vring_t *)malloc(sizeof(vring_t) + size);
    if (ring == NULL) {
        return NULL;
    }

    ring->buf = (unsigned char *)(&ring[1]);
    ring->size = size;
    ring->head = 0;
    ring->tail = 0;

    return ring;
}

/**
 * @brief delete a ring
 *
 * @param ring [IN] the ring to be deleted, must be created by vring_create
 */
void vring_delete(vring_t *ring)
{
    if (ring == NULL) {
        return;
    }
    free(ring);
}

/**
 * @brief put data to a ring, and the free space is enough
 */
static inline size_t __vring_put(vring_t *ring, unsigned char *buf, size_t len)
{
    size_t tail_space;
    size_t mask = ring->size - 1;

    /*
     * tail_space => the space size from tail to the end of the ring
     *               (size - tail & mask)
     */

    tail_space = ring->size - (ring->tail & mask);

    if (len > tail_space) {

        /*
         * must be this condition:
         *
         *        head     tail
         * +------+--------+---------+
         * |      |////////|         |
         * +------+--------+---------+
         *
         * len <= ring_free (already guaranteed in parent function)
         * len >  tail_space
         *
         * so there must be two free partitions, one in the front, one in the end
         */

        memcpy(ring->buf + (ring->tail & mask), buf, tail_space);
        memcpy(ring->buf, buf + tail_space, len - tail_space);
    } else {

        /*
         *
         * may be the following two conditions:
         *
         * 1)
         *        head     tail
         * +------+--------+---------+
         * |      |////////|         |
         * +------+--------+---------+
         *
         * no problem, it's fine:
         *
         *       ring_free >= tail_space >= len
         *
         * 2)
         *        tail     head
         * +------+--------+---------+
         * |//////|        |/////////|
         * +------+--------+---------+
         *
         * there is only one free region, and:
         *
         *       a) len <= ring_free
         *       b) tail_space > ring_free
         *
         * so it is still safe to copy <len> bytes
         */

        memcpy(ring->buf + (ring->tail & mask), buf, len);
    }

    ring->tail += len;

    return len;
}

/**
 * @brief put data to a ring
 *
 * @param ring [IN] the ring which the data will be put in
 * @param buf  [IN] the data buffer
 * @param len  [IN] the length of the data
 *
 * @return the size of data actually put
 */
size_t vring_put(vring_t *ring, unsigned char *buf, size_t len)
{
    return __vring_put(ring, buf, min(len, ring->size - vring_len(ring)));
}

/**
 * @brief put data to a ring forcely
 *
 * @param ring [IN] the ring which the data will be put in
 * @param buf  [IN] the data buffer
 * @param len  [IN] the length of the data
 *
 * @return the size of data actually put
 */
size_t vring_put_force(vring_t *ring, unsigned char *buf, size_t len)
{
    size_t ring_free;

    len = min(len, ring->size);
    ring_free = ring->size - vring_len(ring);

    if (len > ring_free) {
        ring->head += len - ring_free;
    }

    return __vring_put(ring, buf, len);
}

/**
 * @brief optimized version for put one byte to a ring
 *
 * @param ring [IN] the ring which the data will be put in
 * @param byte [IN] the byte to be put in
 *
 * @return the size of data actually put
 */
size_t vring_putc(vring_t *ring, unsigned char byte)
{
    if (vring_full(ring)) {
        return 0;
    }

    ring->buf[ring->tail & (ring->size - 1)] = byte;

    ring->tail++;

    return 1;
}

/**
 * @brief optimized version for put data to a ring forcely
 *
 * @param ring [IN] the ring which the data will be put in
 * @param byte [IN] the byte to be put in
 *
 * @return 1
 */
size_t vring_putc_force(vring_t *ring, unsigned char byte)
{
    if (vring_full(ring)) {
        ring->head++;
    }

    ring->buf[ring->tail & (ring->size - 1)] = byte;

    ring->tail++;

    return 1;
}

/**
 * @brief get data from a ring
 *
 * @param ring [IN] the ring which the data will be put in
 * @param buf  [IN] the data buffer
 * @param len  [IN] the length of the data
 *
 * @return the size of data actually put
 */
size_t vring_get(vring_t *ring, unsigned char *buf, size_t len)
{
    size_t ring_data;
    size_t head_space;
    size_t mask = ring->size - 1;

    /*
     * ring_data  => the data size in the ring
     * head_space => the space size from head to the end of the ring
     *               (size - head & mask)
     */

    ring_data = vring_len(ring);
    head_space = ring->size - (ring->head & mask);

    len = min(len, ring_data);

    if (len > head_space) {

        /*
         * must be this condition:
         *
         *        tail     head
         * +------+--------+---------+
         * |//////|        |/////////|
         * +------+--------+---------+
         *
         * len <= ring_data
         * len >  head_space
         *
         * so there must be two data partitions, one in the front, one in the end
         */

        memcpy(buf, ring->buf + (ring->head & mask), head_space);
        memcpy(buf + head_space, ring->buf, len - head_space);
    } else {

        /*
         * may be the following two conditions:
         *
         * 1)
         *        tail     head
         * +------+--------+---------+
         * |//////|        |/////////|
         * +------+--------+---------+
         *
         * no problem, it's fine:
         *
         *       ring_data >= head_space >= len
         *
         * 2)
         *        head     tail
         * +------+--------+---------+
         * |      |////////|         |
         * +------+--------+---------+
         *
         * there is only one data region, and:
         *
         *       a) len <= ring_data
         *       b) head_space > ring_data
         *
         * so it is still safe to copy <len> bytes
         */

        memcpy(buf, ring->buf + (ring->head & mask), len);
    }

    ring->head += len;

    return len;
}

/**
 * @brief optimized version for get one byte from a ring
 *
 * @param ring [IN] the ring which the data will be put in
 * @param byte [IN] the address where store the byte
 *
 * @return the size of data actually got
 */
size_t vring_getc(vring_t *ring, unsigned char *byte)
{
    if (vring_empty(ring)) {
        return 0;
    }

    *byte = ring->buf[ring->head & (ring->size - 1)];

    ring->head++;

    return 1;
}
