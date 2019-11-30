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

#include "securec.h"

#include <cstdio>
#include <cstring>

#include "gtest/gtest.h"

class TestSecureC : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestSecureC, strncpy_s)
{
    char buf[10] = {0};

    EXPECT_EQ(EOK, strncpy_s(buf, 6, "hello", strlen("hello")));
}