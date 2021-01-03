/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"

#include <climits>

class TestSem : public ::testing::Test
{
protected:
    void SetUp() {
        vos_init();
	};

	void TearDown() {
        vos_exit();
	};
};

static vsem_t  s0;
static vtask_t t1, t2;

static int __task_1(uintptr_t arg)
{
    printf("__task_1 run...arg: 0x%lx\n", arg);
    printf("task 1 will wait on s0\n");
    vsem_wait(&s0);
    printf("task 1 wake up\n");
    vtask_sleep(1);
    printf("__task_1 end...\n");
    return 0;
}

static int __task_2(uintptr_t arg)
{
    printf("__task_2 run...arg: 0x%lx\n", arg);
    printf("task 2 will post s0\n");
    vsem_post(&s0);
    printf("task 2 has post s0 and sleep ~~\n");
    vtask_sleep(5);
    printf("__task_2 end...\n");
    test_quit();
    return 0;
}

TEST_F(TestSem, simple)
{
    vsem_init(&s0, 1, 0);

    test_init();

    vtask_create(&t1, "t1", __task_1, 0x1234, 0x100, 10);
    vtask_create(&t2, "t2", __task_2, 0x4321, 0x100, 11);

    test_wait();

    vsem_destroy(&s0);
    printf("test pass...\n");
}

TEST_F(TestSem, init)
{
    vsem_t s;

    EXPECT_EQ(0, vsem_init(&s, 1, 0));
    EXPECT_EQ(-1, vsem_init(&s, 1, 0)); // repeated

    EXPECT_EQ(0, vsem_destroy(&s));
}

TEST_F(TestSem, destroy)
{
    vsem_t s;

    EXPECT_EQ(-1, vsem_destroy(NULL));
    EXPECT_EQ(-1, vsem_destroy(&s));

    EXPECT_EQ(0, vsem_init(&s, 1, 0));

    EXPECT_EQ(0, vsem_destroy(&s));
    EXPECT_EQ(-1, vsem_destroy(&s));
}

TEST_F(TestSem, wait)
{
    vsem_t s;
    unsigned int val = 0;

    EXPECT_EQ(-1, vsem_wait(&s));

    EXPECT_EQ(0, vsem_init(&s, 1, 1));

    EXPECT_EQ(0, vsem_wait(&s));
    EXPECT_EQ(-1, vsem_trywait(&s));
    EXPECT_EQ(-1, vsem_timedwait(&s, 5));

    EXPECT_EQ(0, vsem_post(&s));
    EXPECT_EQ(0, vsem_post(&s)); // many times to post is ok

    // now value is 2
    EXPECT_EQ(0, vsem_getvalue(&s, &val));
    EXPECT_EQ(2, val);

    EXPECT_EQ(0, vsem_wait(&s));
    EXPECT_EQ(0, vsem_wait(&s));

    EXPECT_EQ(0, vsem_post(&s));
    EXPECT_EQ(0, vsem_destroy(&s));

    EXPECT_EQ(-1, vsem_post(&s));
    EXPECT_EQ(-1, vsem_wait(&s));
}

TEST_F(TestSem, post)
{
    vsem_t s;
    unsigned int val = 0;

    EXPECT_EQ(-1, vsem_post(&s));

    EXPECT_EQ(0, vsem_init(&s, 0, 1));

    EXPECT_EQ(0, vsem_post(&s));
    EXPECT_EQ(0, vsem_getvalue(&s, &val));
    EXPECT_EQ(2, val);

    EXPECT_EQ(0, vsem_destroy(&s));

    EXPECT_EQ(-1, vsem_init(&s, 0, UINT_MAX));
    EXPECT_EQ(EINVAL, errno);
    EXPECT_EQ(0, vsem_init(&s, 0, INT_MAX));

    EXPECT_EQ(-1, vsem_post(&s));
    EXPECT_EQ(0, vsem_getvalue(&s, &val));
    EXPECT_EQ(INT_MAX, val);

    EXPECT_EQ(0, vsem_destroy(&s));
}
