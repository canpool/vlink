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

#include "vsl_pbkdf2.h"

#include <sys/time.h>

#include "gtest/gtest.h"

class TestPbkdf2 : public ::testing::Test
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

TEST_F(TestPbkdf2, pbkdf2)
{
	unsigned char *pwd = (unsigned char *)"123456789abc";
    unsigned char *salt = (unsigned char *)"abcd1234";
	unsigned char output[64] = {0};

	vsl_pbkdf2(pwd, strlen((char *)pwd),
        salt, strlen((char *)salt), 1, 16, output);
	printf("hmac16:\n");
	print_array(output, 16);

    vsl_pbkdf2(pwd, strlen((char *)pwd),
        salt, strlen((char *)salt), 1, 32, output);
	printf("hmac32:\n");
	print_array(output, 32);

    vsl_pbkdf2(pwd, strlen((char *)pwd),
        salt, strlen((char *)salt), 1, 64, output);
	printf("hmac64:\n");
	print_array(output, 64);
}
