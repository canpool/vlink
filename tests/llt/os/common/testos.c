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

#include "testos.h"
#include "vos.h"

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
        vos_task_sleep(10);
    }
    s_quit_flag = 0;
}