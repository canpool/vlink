/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"
#include "vconfig.h"

class TestTimer : public ::testing::Test
{
protected:
    void SetUp() {
        vos_init();
	};

	void TearDown() {
        vos_exit();
	};
};

#ifdef CONFIG_SWTMR

static volatile int once = 0;
static volatile int perd = 0;

static void __timer_once(uintptr_t arg)
{
    printf("%d in __timer_once(0x%x)\n", ++once, (uint32_t)arg);
}

static void __timer_perd(uintptr_t arg)
{
    printf("%d in __timer_perd(0x%x)\n", ++perd, (uint32_t)arg);
}

TEST_F(TestTimer, create)
{
    vtimer_t t1, t2, t3;

    EXPECT_EQ(-1, vtimer_create(&t1, 10, V_TIMER_MODE_ONESHOT, NULL, 0));

    EXPECT_EQ(0, vtimer_create(&t1, 10, V_TIMER_MODE_ONESHOT, __timer_once, 0));
    EXPECT_EQ(0, vtimer_create(&t2, 10, V_TIMER_MODE_ONESHOT, __timer_once, 0));
    EXPECT_EQ(0, vtimer_create(&t3, 10, V_TIMER_MODE_ONESHOT, __timer_once, 0));

    // do not delete in order
    EXPECT_EQ(0, vtimer_delete(&t3));
    EXPECT_EQ(0, vtimer_delete(&t1));
    EXPECT_EQ(0, vtimer_delete(&t2));

    EXPECT_EQ(-1, vtimer_delete(&t1)); // repeated
}

TEST_F(TestTimer, delete)
{
    vtimer_t t;

    EXPECT_EQ(-1, vtimer_delete(NULL));
    EXPECT_EQ(-1, vtimer_delete(&t));

    EXPECT_EQ(0, vtimer_create(&t, 10, V_TIMER_MODE_ONESHOT, __timer_once, 0));

    EXPECT_EQ(0, vtimer_delete(&t));
}

TEST_F(TestTimer, start)
{
    vtimer_t t1, t2;

    EXPECT_EQ(-1, vtimer_start(NULL));
    EXPECT_EQ(-1, vtimer_start(&t1));

    EXPECT_EQ(0, vtimer_create(&t1, 30, V_TIMER_MODE_ONESHOT, __timer_once, 0x1234));
    EXPECT_EQ(0, vtimer_create(&t2, 40, V_TIMER_MODE_REPEATED, __timer_perd, 0x4321));

    EXPECT_EQ(0, vtimer_start(&t1));
    EXPECT_EQ(0, vtimer_start(&t2));

    vtask_sleep(100);
    EXPECT_EQ(1, once);
    EXPECT_EQ(2, perd);
    once = perd = 0;

    EXPECT_EQ(0, vtimer_delete(&t1));
    EXPECT_EQ(0, vtimer_delete(&t2));
}

TEST_F(TestTimer, stop)
{
    vtimer_t t1, t2;

    EXPECT_EQ(-1, vtimer_stop(NULL));
    EXPECT_EQ(-1, vtimer_stop(&t1));

    EXPECT_EQ(0, vtimer_create(&t1, 30, V_TIMER_MODE_ONESHOT, __timer_once, 0x1234));
    EXPECT_EQ(0, vtimer_start(&t1));

    vtask_sleep(20);
    EXPECT_EQ(0, vtimer_stop(&t1));
    vtask_sleep(10);
    EXPECT_EQ(0, once);

    EXPECT_EQ(0, vtimer_start(&t1)); // restart, recount
    vtask_sleep(20);
    EXPECT_EQ(0, once);
    vtask_sleep(10);
    EXPECT_EQ(1, once);
    EXPECT_EQ(0, vtimer_delete(&t1));

    EXPECT_EQ(0, vtimer_create(&t2, 40, V_TIMER_MODE_REPEATED, __timer_perd, 0x4321));
    EXPECT_EQ(0, vtimer_start(&t2));

    vtask_sleep(40); // critical
    EXPECT_EQ(1, perd);
    EXPECT_EQ(0, vtimer_stop(&t2));
    EXPECT_EQ(0, vtimer_start(&t2)); // restart, recount
    vtask_sleep(60);
    EXPECT_EQ(2, perd);
    EXPECT_EQ(0, vtimer_delete(&t2));
    vtask_sleep(30);
    EXPECT_EQ(2, perd); // still 2

    once = perd = 0;
}

#endif
