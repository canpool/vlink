/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vring.h"

#include <cstring>

class TestRing : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestRing, init)
{
    vring_t ring;
    unsigned char buf[32] = {0};
    memset(&ring, 0, sizeof(vring_t));

    EXPECT_EQ(-1, vring_init(NULL, NULL, 0));
    EXPECT_EQ(-1, vring_init(&ring, NULL, 0));
    EXPECT_EQ(-1, vring_init(&ring, buf, 0));
    EXPECT_EQ(-1, vring_init(&ring, buf, 10));
    EXPECT_EQ( 0, vring_init(&ring, buf, sizeof(buf)));
}

TEST_F(TestRing, create)
{
    vring_t *ring = NULL;

    EXPECT_FALSE(vring_create(0));
    EXPECT_FALSE(vring_create(10));
    ASSERT_TRUE(ring = vring_create(32));

    EXPECT_NO_THROW(vring_delete(ring));
    EXPECT_NO_THROW(vring_delete(NULL));
}

TEST_F(TestRing, put)
{
    vring_t ring;
    unsigned char buf[4] = {0};
    unsigned char data[] = "ring";

    EXPECT_EQ(0, vring_init(&ring, buf, sizeof(buf)));

    EXPECT_EQ(4, vring_put(&ring, data, sizeof(data)));
    EXPECT_EQ(0, vring_put(&ring, data, sizeof(data)));

    EXPECT_EQ(0, vring_putc(&ring, 'v'));
    EXPECT_EQ(1, vring_putc_force(&ring, 'v'));
    EXPECT_EQ(0, memcmp("ving", buf, sizeof(buf)));

    EXPECT_EQ(4, vring_put_force(&ring, data, sizeof(data)));
    EXPECT_EQ(0, memcmp("grin", buf, sizeof(buf)));
}

TEST_F(TestRing, get)
{
    vring_t ring;
    unsigned char buf[4] = {0};
    unsigned char data[] = "ring";

    EXPECT_EQ(0, vring_init(&ring, buf, sizeof(buf)));

    EXPECT_EQ(4, vring_put(&ring, data, sizeof(data)));

    unsigned char byte = 0;
    unsigned char tmp[5] = {0};

    EXPECT_EQ(4, vring_get(&ring, tmp, sizeof(tmp)));
    EXPECT_EQ(0, vring_get(&ring, tmp, sizeof(tmp)));

    EXPECT_EQ(0, vring_getc(&ring, &byte));

    EXPECT_EQ(4, vring_put(&ring, data, sizeof(data)));

    EXPECT_EQ(1, vring_getc(&ring, &byte));
    EXPECT_EQ('r', byte);

    EXPECT_EQ(1, vring_putc(&ring, 'v'));
    EXPECT_EQ(1, vring_getc(&ring, &byte));
    EXPECT_EQ('i', byte);

    EXPECT_EQ(3, vring_get(&ring, tmp, sizeof(tmp)));
    EXPECT_EQ(0, memcmp("ngv", tmp, 3));
}

TEST_F(TestRing, demo)
{
    vring_t ring;
    unsigned char buf[4] = {0};
    unsigned char data[] = "ring";

    EXPECT_EQ(0, vring_init(&ring, buf, sizeof(buf)));

    EXPECT_EQ(4, vring_put(&ring, data, sizeof(data)));
    EXPECT_TRUE(vring_full(&ring));

    unsigned char byte = 0;
    unsigned char tmp[5] = {0};

    EXPECT_EQ(4, vring_get(&ring, tmp, sizeof(tmp)));
    EXPECT_TRUE(vring_empty(&ring));

    EXPECT_EQ(1, vring_putc(&ring, 'v'));
    EXPECT_EQ(1, vring_len(&ring));

    EXPECT_EQ(3, vring_put(&ring, data, sizeof(data)));
    EXPECT_EQ(0, memcmp("vrin", buf, sizeof(buf)));

    EXPECT_EQ('v', vring_peek(&ring, 0));
    EXPECT_EQ('n', vring_peek(&ring, 3));
    EXPECT_EQ('v', vring_peek(&ring, 4));
}
