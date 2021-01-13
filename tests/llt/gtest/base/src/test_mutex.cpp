/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"

#include "verrno.h"

class TestMutex : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static vmutex_t m0;
static vtask_t  t1, t2;

static int __task_1(uintptr_t arg)
{
    printf("__task_1 run...\n");
    vmutex_lock(&m0);
    printf("task 1 lock and sleep ~~\n");
    vtask_sleep(1);
    vmutex_unlock(&m0);
    printf("task 1 unlock and sleep ~~\n");
    vtask_sleep(1);
    printf("__task_1 end...\n");

    return 0;
}

static int __task_2(uintptr_t arg)
{
    printf("__task_2 run...\n");
    vmutex_lock(&m0);
    printf("task 2 hold mutex..\n");
    vmutex_unlock(&m0);
    printf("task 2 unlock and sleep ~~\n");
    vtask_sleep(5);
    printf("__task_2 end...\n");

    test_quit();

    return 0;
}

TEST_F(TestMutex, simple)
{
    vmutex_init(&m0);

    test_init();

    vtask_create(&t1, "t1", __task_1, 0, 0x100, 10);
    vtask_create(&t2, "t2", __task_2, 0, 0x100, 11);

    test_wait();

    vmutex_destroy(&m0);
    vtask_sleep(100);

    printf("test pass...\n");
}

TEST_F(TestMutex, init)
{
    vmutex_t m;

    EXPECT_EQ(0, vmutex_init(&m));
    EXPECT_EQ(-1, vmutex_init(&m));

    EXPECT_EQ(0, vmutex_destroy(&m));
}

TEST_F(TestMutex, destroy)
{
    vmutex_t m;

    EXPECT_EQ(0, vmutex_init(&m));

    EXPECT_EQ(0, vmutex_destroy(&m));
    EXPECT_EQ(-1, vmutex_destroy(&m));
}

TEST_F(TestMutex, lock)
{
    vmutex_t m;

    EXPECT_EQ(-1, vmutex_lock(NULL));
    EXPECT_EQ(-1, vmutex_lock(&m));

    EXPECT_EQ(0, vmutex_init(&m));

    EXPECT_EQ(0, vmutex_lock(&m));
    EXPECT_EQ(0, vmutex_unlock(&m));

    EXPECT_EQ(0, vmutex_unlock(&m)); // note: need lock
    EXPECT_EQ(0, vmutex_lock(&m));

    EXPECT_EQ(0, vmutex_destroy(&m));

    EXPECT_EQ(-1, vmutex_unlock(&m));
    EXPECT_EQ(-1, vmutex_lock(&m));
}

TEST_F(TestMutex, trylock)
{
    vmutex_t m;

    EXPECT_EQ(-1, vmutex_trylock(NULL));
    EXPECT_EQ(-1, vmutex_trylock(&m));

    EXPECT_EQ(0, vmutex_init(&m));

    EXPECT_EQ(0, vmutex_trylock(&m));
    EXPECT_EQ(EBUSY, vmutex_trylock(&m)); // note: need unlock
    EXPECT_EQ(0, vmutex_unlock(&m));

    EXPECT_EQ(0, vmutex_unlock(&m));
    EXPECT_EQ(0, vmutex_trylock(&m));

    EXPECT_EQ(0, vmutex_destroy(&m));

    EXPECT_EQ(-1, vmutex_unlock(&m));
    EXPECT_EQ(-1, vmutex_trylock(&m));
}

TEST_F(TestMutex, timedlock)
{
    vmutex_t m;

    EXPECT_EQ(-1, vmutex_timedlock(NULL, 0));
    EXPECT_EQ(-1, vmutex_timedlock(&m, 0));

    EXPECT_EQ(0, vmutex_init(&m));

    EXPECT_EQ(0, vmutex_timedlock(&m, 10));
    EXPECT_EQ(ETIMEDOUT, vmutex_timedlock(&m, 10)); // note: need unlock
    EXPECT_EQ(0, vmutex_unlock(&m));

    EXPECT_EQ(0, vmutex_unlock(&m));
    EXPECT_EQ(0, vmutex_timedlock(&m, 10));

    EXPECT_EQ(0, vmutex_destroy(&m));

    EXPECT_EQ(-1, vmutex_unlock(&m));
    EXPECT_EQ(-1, vmutex_timedlock(&m, 10));
}

TEST_F(TestMutex, unlock)
{
    vmutex_t m;

    EXPECT_EQ(-1, vmutex_unlock(NULL));
    EXPECT_EQ(-1, vmutex_unlock(&m));

    EXPECT_EQ(0, vmutex_init(&m));
    EXPECT_EQ(0, vmutex_unlock(&m)); // note: need lock

    EXPECT_EQ(EBUSY, vmutex_destroy(&m));
    vtask_sleep(10); // wait
    EXPECT_EQ(0, vmutex_lock(&m));
    EXPECT_EQ(0, vmutex_destroy(&m));
}
