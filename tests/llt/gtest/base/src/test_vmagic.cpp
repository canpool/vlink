/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vmagic.h"

class TestMagic : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestMagic, demo)
{
    vmagic_t m;

    EXPECT_FALSE(vmagic_verify(&m));

    vmagic_set(&m, 0x10);

    EXPECT_EQ(0x10, vmagic_get(&m));

    EXPECT_TRUE(vmagic_verify(&m));

    vmagic_reset(&m);

    EXPECT_FALSE(vmagic_verify(&m));
}
