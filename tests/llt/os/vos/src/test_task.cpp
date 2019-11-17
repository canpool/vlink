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

#include "vos.h"
#include "testos.h"

#include "gtest/gtest.h"

class TestTask : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static vmutex_t m0;
static vtask_t t1, t2;

static int __task_1(uintptr_t arg)
{
    vprintf("__task_1 run...\n");
}

static int __task_2(uintptr_t arg)
{
    vprintf("__task_2 run...\n");
}

TEST_F(TestTask, simple)
{
    vos_mutex_init(&m0);

    vos_task_create(&t1, "t1", __task_1, 0, 0x100, 10);
    vos_task_create(&t2, "t2", __task_2, 0, 0x100, 10);

    vos_task_sleep(10);
    vprintf("test pass...\n");

    vos_mutex_destroy(&m0);
}
