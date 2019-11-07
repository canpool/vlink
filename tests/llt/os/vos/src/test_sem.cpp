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

class TestSem : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static vsem_t s0;
static vtask_t t1, t2;

static int __task_1(uintptr_t arg)
{
    vprintf("__task_1 run...arg: 0x%x\n", arg);
    vprintf("task 1 will wait on s0\n");
    vos_sem_wait(s0);
    vprintf("task 1 wake up\n");
    vos_task_sleep(1);
    vprintf("__task_1 end...\n");
}

static int __task_2(uintptr_t arg)
{
    vprintf("__task_2 run...arg: 0x%x\n", arg);
    vprintf("task 2 will post s0\n");
    vos_sem_post(s0);
    vprintf("task 2 has post s0 and sleep ~~\n");
    vos_task_sleep(5);
    vprintf("__task_2 end...\n");
    test_quit();
}

TEST_F(TestSem, simple)
{
    vos_sem_init(&s0, 1, 0);

    test_init();

    vos_task_create(&t1, "t1", __task_1, 0x1234, 0x100, 10);
    vos_task_create(&t2, "t2", __task_2, 0x4321, 0x100, 11);

    test_wait();

    vprintf("test pass...\n");

    vos_sem_destroy(s0);
}