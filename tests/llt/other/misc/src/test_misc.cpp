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

#include "vmisc.h"

#include <cstdio>
#include <cstring>

#include "gtest/gtest.h"

class TestMisc : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestMisc, hexify)
{
    unsigned char s1[3] = {0xAB, 0x1a, 0x23};
    char d1[7] = {0};
    char *s2 = d1;
    unsigned char d2[3] = {0};

    EXPECT_EQ(0, v_hexify(s1, sizeof(s1), d1, sizeof(d1), 0));
    printf("d1: %s\n", d1);
    EXPECT_EQ(0, v_hexify(s1, sizeof(s1), d1, sizeof(d1), 1));
    printf("d1: %s\n", d1);

    EXPECT_EQ(0, v_unhexify(s2, strlen(s2), d2, sizeof(d2)));
    for (int i = 0; i < sizeof(s1); ++i) {
        printf("s1[%d]: 0x%x, d2[%d]: 0x%x\n", i, s1[i], i, d2[i]);
    }
    EXPECT_EQ(0, memcmp(s1, d2, sizeof(s1)));
}