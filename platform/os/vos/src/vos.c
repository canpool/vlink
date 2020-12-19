/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vos.h"

#include <string.h>

void *vmem_zalloc(size_t size)
{
    void *ptr = vmem_malloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void *vmem_calloc(size_t n, size_t size)
{
    void *ptr = vmem_malloc(n * size);
    if (ptr != NULL) {
        memset(ptr, 0, n * size);
    }
    return ptr;
}

void msleep(uint32_t ms)
{
    uint64_t start_ms = mseconds();

    while (start_ms + ms > mseconds()) {
        /* do nothing */
    }
}

/* mock */

v_weak void *vmem_malloc(size_t size)
{
    return NULL;
}

v_weak uint64_t mseconds(void)
{
    return 0;
}