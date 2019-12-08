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

#include "vsl_cmac.h"

#include <sys/time.h>

#include "gtest/gtest.h"

class TestCmac : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static void print_array(unsigned char *array, int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		printf("0x%02X ", array[i]);
		if ((i + 1) % 8 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

TEST_F(TestCmac, cmac)
{
	unsigned char *key = (unsigned char *)"123456789abc";
	unsigned char *input = (unsigned char *)"hello cmac";
	unsigned char output[16] = {0};

	vsl_cmac(key, strlen((char *)key), input, strlen((char *)input), output);
	printf("cmac:\n");
	print_array(output, 16);
}
