/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"

class TestTask : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static vtask_t       t1, t2;
static vtask_t       t3, t4;
static volatile bool q_flag = false;

static int __task_test(uintptr_t arg)
{
    printf("arg: 0x%x\n", (uint32_t)arg);
    return 0;
}

static int __task_1(uintptr_t arg)
{
    int i = 0;

    for (;;) {
        printf("__task_1 run...%d\n", i++);
        vtask_sleep(20);
    }
    return 0;
}

static int __task_2(uintptr_t arg)
{
    int i = 0;

    for (;;) {
        printf("__task_2 run...%d\n", i++);
        vtask_sleep(20);
    }
    return 0;
}

static int __task_3(uintptr_t arg)
{
    printf("__task_3 natural exit\n");
    return 0;
}

static int __task_4(uintptr_t arg)
{
    int i = 0;

    for (;;) {
        printf("__task_4 run...%d\n", i++);
        vtask_sleep(20);

        if (q_flag) {
            vtask_exit();
        }
    }
    return 0;
}

static int __task_5(uintptr_t arg)
{
    for (;;) {
        vtask_lock();
        printf("__task_5 run\n");
        vtask_sleep(5);
        printf("__task_5 arg...0x%x\n", (uint32_t)arg);
        vtask_unlock();
        vtask_sleep(5);
    }
    return 0;
}

TEST_F(TestTask, create)
{
    vtask_t t;

    EXPECT_EQ(-1, vtask_create(&t, "test", NULL, 0, 0x100, 1));
    EXPECT_EQ( 0, vtask_create(&t, "test", __task_test, 0x1234, 0x100, 1));
    EXPECT_EQ(-1, vtask_create(&t, "test", __task_test, 0x4321, 0x100, 1));
    EXPECT_NO_THROW(vtask_sleep(10));
}

TEST_F(TestTask, delete)
{
    vtask_t t;

    EXPECT_EQ(-1, vtask_delete(NULL));
    EXPECT_EQ(-1, vtask_delete(&t));

    EXPECT_EQ(0, vtask_create(&t, "test", __task_1, 0, 0x100, 10));
    EXPECT_EQ(0, vtask_delete(&t));

    EXPECT_EQ(-1, vtask_delete(&t)); // repeated
}

TEST_F(TestTask, switch)
{
    vtask_create(&t1, "t1", __task_1, 0, 0x100, 10);
    vtask_create(&t2, "t2", __task_2, 0, 0x100, 11);

    vtask_sleep(100);

    vtask_delete(&t1);
    vtask_delete(&t2);

    vtask_sleep(100); // wait a moment
    printf("test pass...\n");
}

TEST_F(TestTask, compete)
{
    vtask_create(&t1, "t1", __task_5, 0x1234, 0x100, 10);
    vtask_create(&t2, "t2", __task_5, 0x4321, 0x100, 11);

    vtask_sleep(20);

    vtask_delete(&t1);
    vtask_delete(&t2);

    vtask_sleep(100); // wait a moment
    printf("test pass...\n");
}

TEST_F(TestTask, natural_exit)
{
    vtask_create(&t3, "t3", __task_3, 0, 0x100, 10);

    vtask_sleep(100);
    printf("test pass...\n");
}

TEST_F(TestTask, exit)
{
    vtask_create(&t4, "t4", __task_4, 0, 0x100, 10);

    vtask_sleep(100);
    q_flag = true;
    vtask_sleep(100);
    printf("test pass...\n");
}

TEST_F(TestTask, misc)
{
    vtask_resume(NULL);
    vtask_suspend(NULL);

    vtask_prio_set(NULL, 0);
    vtask_prio_get(NULL);

    vtask_stat();

    vtask_waterline(NULL);
}
