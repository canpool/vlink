/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"

#define VLOG_TAG "TestLog"
#include "vlog.h"

#include "vos.h"

class TestLog : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestLog, demo)
{
    EXPECT_EQ(VLOG_INFO, vlog_get_level());
    vlog_debug("debug");
    vlog_info("info");

    EXPECT_NO_THROW(vlog_set_level(VLOG_DEBUG));
    EXPECT_EQ(VLOG_DEBUG, vlog_get_level());
    vlog_debug("debug");
    vlog_info("info");
    vtask_sleep(10);
    vlog_error("error");
}
