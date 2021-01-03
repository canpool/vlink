/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"

class TestEvent : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestEvent, misc)
{
    uint32_t recved = 0;

    vevent_init(NULL);

    vevent_destroy(NULL);

    vevent_timedrecv(NULL, 0, 0, 0, &recved);
    vevent_tryrecv(NULL, 0, 0, &recved);
    vevent_recv(NULL, 0, 0, &recved);

    vevent_send(NULL, 0);

    vevent_clear(NULL, 0);
}
