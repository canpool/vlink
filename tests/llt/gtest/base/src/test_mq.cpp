/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "testos.h"

#include "vconfig.h"
#include "verrno.h"

class TestMQ : public ::testing::Test
{
protected:
    void SetUp() {
        vos_init();
	};

	void TearDown() {
        vos_exit();
	};
};

TEST_F(TestMQ, simple)
{
    vmq_t  mq;
    char   buf[20]  = {0};
    size_t len      = sizeof(buf);
    int    recv_len = 0;

    if (vmq_create(&mq, "/testmq", 20, 5, 0) != 0) {
        printf("vmq_create() failed, errno: %d\n", errno);
        return;
    }

    if (vmq_send(&mq, "hello", strlen("hello")) != 0) {
        printf("vmq_send() failed, errno: %d\n", errno);
    } else {
        recv_len = vmq_recv(&mq, buf, len);
        printf("want recv len: %lu, real recv len: %d\n", len, recv_len);
        if (recv_len > 0) {
            ASSERT_EQ(strlen("hello"), recv_len);
        }
    }

    vmq_delete(&mq);
}

TEST_F(TestMQ, create)
{
    vmq_t q;

    EXPECT_EQ(-1, vmq_create(&q, NULL, sizeof(vmq_buf_t), 2, 0));
    ASSERT_EQ(0, vmq_create(&q, "/qc", sizeof(vmq_buf_t), 2, 0));
    EXPECT_EQ(-1, vmq_create(&q, "/qc", sizeof(vmq_buf_t), 2, 0));

    EXPECT_EQ(0, vmq_delete(&q));
}

TEST_F(TestMQ, delete)
{
    vmq_t q;

    EXPECT_EQ(-1, vmq_delete(NULL));
    EXPECT_EQ(-1, vmq_delete(&q));

    ASSERT_EQ(0, vmq_create(&q, "/qd", sizeof(vmq_buf_t), 2, 0));

    EXPECT_EQ(0, vmq_delete(&q));
    EXPECT_EQ(-1, vmq_delete(&q));
}

TEST_F(TestMQ, send)
{
    vmq_t q;
    vmq_buf_t buf, tmp;
    int ret = 0;

    ASSERT_EQ(0, vmq_create(&q, "/qs", sizeof(vmq_buf_t), 2, 0));

    buf.len = 10;
    buf.buf = (uint8_t *)vmem_zalloc(buf.len);
    strcpy((char *)buf.buf, "hello");
    EXPECT_EQ(0, vmq_send(&q, &buf, sizeof(vmq_buf_t)));

    buf.len = 10;
    buf.buf = (uint8_t *)vmem_zalloc(buf.len);
    strcpy((char *)buf.buf, "world");
    EXPECT_EQ(0, vmq_send(&q, &buf, sizeof(vmq_buf_t)));

    buf.len = 10;
    buf.buf = (uint8_t *)vmem_zalloc(buf.len);
    strcpy((char *)buf.buf, "msgQ");
    ret = vmq_trysend(&q, &buf, sizeof(vmq_buf_t));
#ifdef CONFIG_SWQUE
    EXPECT_EQ(VEFULL, ret);
#else
    EXPECT_NE(0, ret);
#endif
    ret = vmq_timedsend(&q, &buf, sizeof(vmq_buf_t), 10);
#ifdef CONFIG_SWQUE
    EXPECT_EQ(VEFULL, ret);
#else
    EXPECT_NE(0, ret);
#endif

    EXPECT_EQ(0, vmq_recv(&q, &tmp, sizeof(vmq_buf_t)));
    EXPECT_EQ(10, tmp.len);
    EXPECT_TRUE(0 == memcmp(tmp.buf, "hello", strlen("hello")));
    vmem_free(tmp.buf);

    EXPECT_EQ(0, vmq_trysend(&q, &buf, sizeof(vmq_buf_t)));

    EXPECT_EQ(0, vmq_recv(&q, &tmp, sizeof(vmq_buf_t)));
    EXPECT_EQ(10, tmp.len);
    EXPECT_TRUE(0 == memcmp(tmp.buf, "world", strlen("world")));
    vmem_free(tmp.buf);

    EXPECT_EQ(0, vmq_recv(&q, &tmp, sizeof(vmq_buf_t)));
    EXPECT_EQ(10, tmp.len);
    EXPECT_TRUE(0 == memcmp(tmp.buf, "msgQ", strlen("msgQ")));
    vmem_free(tmp.buf);

    EXPECT_EQ(0, vmq_delete(&q));
}

TEST_F(TestMQ, recv)
{
    vmq_t q;
    vmq_buf_t buf, tmp;
    int ret = 0;

    ASSERT_EQ(0, vmq_create(&q, "/qr", sizeof(vmq_buf_t), 1, 1));

    buf.len = 10;
    buf.buf = (uint8_t *)vmem_zalloc(buf.len);
    strcpy((char *)buf.buf, "hello");
    EXPECT_EQ(0, vmq_send(&q, &buf, sizeof(vmq_buf_t)));

    buf.len = 10;
    buf.buf = (uint8_t *)vmem_zalloc(buf.len);
    strcpy((char *)buf.buf, "msgQ");
    ret = vmq_trysend(&q, &buf, sizeof(vmq_buf_t));
#ifdef CONFIG_SWQUE
    EXPECT_EQ(VEFULL, ret);
#else
    EXPECT_NE(0, ret);
#endif

    EXPECT_EQ(0, vmq_recv(&q, &tmp, sizeof(vmq_buf_t)));
    EXPECT_EQ(10, tmp.len);
    EXPECT_TRUE(0 == memcmp(tmp.buf, "hello", strlen("hello")));
    vmem_free(tmp.buf);

    EXPECT_EQ(0, vmq_trysend(&q, &buf, sizeof(vmq_buf_t)));

    EXPECT_EQ(0, vmq_recv(&q, &tmp, sizeof(vmq_buf_t)));
    EXPECT_EQ(10, tmp.len);
    EXPECT_TRUE(0 == memcmp(tmp.buf, "msgQ", strlen("msgQ")));
    vmem_free(tmp.buf);

    ret = vmq_tryrecv(&q, &buf, sizeof(vmq_buf_t));
#ifdef CONFIG_SWQUE
    EXPECT_EQ(VEEMPTY, ret);
#else
    EXPECT_NE(0, ret);
#endif
    ret = vmq_timedrecv(&q, &buf, sizeof(vmq_buf_t), 10);
#ifdef CONFIG_SWQUE
    EXPECT_EQ(VEEMPTY, ret);
#else
    EXPECT_NE(0, ret);
#endif

    EXPECT_EQ(0, vmq_delete(&q));
}

