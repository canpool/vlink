/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vbug.h"
#include "vcompiler.h"

volatile int __vbug_dummy_check = 0;

v_weak void vbug(const char *info)
{
    if (info[0] != '\0') {
        uprintf("BUG, \"%s\"\r\n", info);
    }

    while (__vbug_dummy_check == 0) {
        /* code */
    }
}
