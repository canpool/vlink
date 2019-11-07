/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "vos.h"
#include "testos.h"

#include <errno.h>

#include "gtest/gtest.h"

class TestMQ : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestMQ, simple)
{
    vmq_t mq;
    char buf[20] = {0};
    size_t len = sizeof(buf);
    int recv_len = 0;

    if (vos_mq_create(&mq, "/testmq", 20, 5, 0) != 0) {
        vprintf("vos_mq_create() failed, errno: %d\n", errno);
        return;
    }

    if (vos_mq_send(mq, "hello", strlen("hello")) != 0) {
        vprintf("vos_mq_send() failed, errno: %d\n", errno);
    } else {
        recv_len = vos_mq_recv(mq, buf, len);
        vprintf("want recv len: %d, real recv len: %d\n", len, recv_len);
        if (recv_len > 0) {
            ASSERT_EQ(strlen("hello"), recv_len);
        }
    }

    vos_mq_delete(mq);
}