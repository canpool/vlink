/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "hal.h"

class TestHal : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

#define BUF_LEN 20
static uint8_t s_buf[BUF_LEN] = {0};

static int test_open(hal_dev_t *dev, int oflag, va_list args)
{
    return 0;
}

static int test_close(hal_dev_t *dev)
{
    return 0;
}

static size_t test_read(hal_dev_t *dev, int pos, void *buf, size_t len)
{
    size_t rlen = 0;

    if (pos < 0 || pos >= BUF_LEN) {
        return rlen;
    }
    if (pos + len > BUF_LEN) {
        rlen = BUF_LEN - pos;
    } else {
        rlen = len;
    }
    memcpy(buf, s_buf + pos, rlen);

    return rlen;
}

static size_t test_write(hal_dev_t *dev, int pos, const void *buf, size_t len)
{
    size_t wlen = 0;

    if (pos < 0 || pos >= BUF_LEN) {
        return wlen;
    }
    if (pos + len > BUF_LEN) {
        wlen = BUF_LEN - pos;
    } else {
        wlen = len;
    }
    memcpy(s_buf + pos, buf, wlen);

    return wlen;
}

static int test_ioctl(hal_dev_t *dev, int cmd, va_list args)
{
    return 0;
}

static hal_dev_ops_t test_ops = {
    test_open,
    test_close,
    test_read,
    test_write,
    test_ioctl
};

static hal_dev_t test_dev;

TEST_F(TestHal, demo)
{
    EXPECT_EQ(VEINVAL, hal_register(NULL));
    EXPECT_EQ(VEINVAL, hal_register(&test_dev));

    EXPECT_EQ(VEINVAL, hal_unregister(NULL));
    EXPECT_EQ(VEINVAL, hal_unregister(&test_dev));

    test_dev.name = "test";
    EXPECT_EQ(VOK, hal_register(&test_dev));

    hal_dev_t *dev = NULL;
    EXPECT_TRUE((dev = hal_find(NULL)) == NULL);
    EXPECT_TRUE((dev = hal_find("t")) == NULL);
    EXPECT_TRUE((dev = hal_find("test")) != NULL);
    EXPECT_TRUE(dev == &test_dev);

    EXPECT_TRUE((dev = hal_open(NULL, 0)) == NULL);
    EXPECT_TRUE((dev = hal_open("t", 0)) == NULL);
    EXPECT_TRUE((dev = hal_open("test", 0)) != NULL);

    uint8_t buf[10] = {0};

    EXPECT_EQ(VEINVAL, hal_read(NULL, 0, NULL, 0));
    EXPECT_EQ(VEINVAL, hal_read(dev, 0, NULL, 0));
    EXPECT_EQ(VEINVAL, hal_read(dev, 0, buf, 0));
    EXPECT_EQ(VEPERM, hal_read(dev, 0, buf, 10));

    EXPECT_EQ(VEINVAL, hal_write(NULL, 0, NULL, 0));
    EXPECT_EQ(VEINVAL, hal_write(dev, 0, NULL, 0));
    EXPECT_EQ(VEINVAL, hal_write(dev, 0, buf, 0));
    EXPECT_EQ(VEPERM, hal_write(dev, 0, buf, 10));

    EXPECT_EQ(VEINVAL, hal_ioctl(NULL, 0));
    EXPECT_EQ(VEPERM, hal_ioctl(dev, 0));

    EXPECT_EQ(VEINVAL, hal_close(NULL));
    EXPECT_EQ(VEPERM, hal_close(dev));

    test_dev.ops = &test_ops;

    EXPECT_EQ(5, hal_write(dev, 0, "hello", 5));
    EXPECT_EQ(5, hal_write(dev, 5, "world", 5));
    EXPECT_EQ(10, hal_read(dev, 0, buf, 10));
    EXPECT_EQ(0, memcmp(buf, "helloworld", 10));

    EXPECT_EQ(0, hal_ioctl(dev, 0));

    EXPECT_EQ(0, hal_close(dev));
    EXPECT_EQ(0, hal_close(dev));

    EXPECT_EQ(VOK, hal_unregister(&test_dev));
    EXPECT_EQ(VOK, hal_unregister(&test_dev));
}
