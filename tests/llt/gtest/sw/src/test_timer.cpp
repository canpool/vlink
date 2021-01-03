/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "swtmr.h"
#include "vos.h"

#include <cstdio>
#include <cstring>

class TestSwtmr : public ::testing::Test
{
protected:
    void SetUp() {
        vos_init();
	};

	void TearDown() {
        vos_exit();
	};
};

static void time1(uintptr_t arg)
{
    printf("time1...");
    if (arg == 0x1234) {
        printf("0x%x", (uint32_t)arg);
    }
    printf("...%lu\n", vsystime());
}

static void time2(uintptr_t arg)
{
    printf("time2...");
    if (arg == 0x4321) {
        printf("0x%x", (uint32_t)arg);
    }
    printf("...%lu\n", vsystime());
}

TEST_F(TestSwtmr, demo)
{
    swtmr_t *s1 = swtmr_create("time1", 50, SWTMR_MODE_ONCE,   time1, 0x1234);
    swtmr_t *s2 = swtmr_create("time2", 10, SWTMR_MODE_PERIOD, time2, 0x4321);

    swtmr_start(s2);
    swtmr_start(s1);

    vtask_sleep(100);

    swtmr_stop(s1);
    swtmr_stop(s2);

    swtmr_delete(s1);
    swtmr_delete(s2);
}
