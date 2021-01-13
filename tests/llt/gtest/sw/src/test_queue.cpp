/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "swque.h"
#include "vos.h"

#include <cstdio>
#include <cstring>

class TestSwque : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestSwque, demo)
{
    swque_t *q = swque_create("q1", 10, 2, 0);
    char buf[10] = {0};

    swque_push(q, "hello", 6, 0);
    printf("push: hello\n");
    swque_pop(q, buf, sizeof(buf), 0);

    printf("pop : %s\n", buf);

    swque_delete(q);
}
