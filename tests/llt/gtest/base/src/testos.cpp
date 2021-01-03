/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "testos.h"

static int s_quit_flag = 0;

void test_init(void)
{
    s_quit_flag = 0;
}

void test_quit(void)
{
    s_quit_flag = 1;
}

void test_wait(void)
{
    while (!s_quit_flag) {
        vtask_sleep(10);
    }
    s_quit_flag = 0;
}