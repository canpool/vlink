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

#include "ecdsa256.h"

#include <sys/time.h>

#include "gtest/gtest.h"

class TestEcdsa256 : public ::testing::Test
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
		printf("%02x ", array[i]);
		if ((i + 1) % 20 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

TEST_F(TestEcdsa256, init)
{
	EXPECT_EQ(0, ecdsa_init());
	EXPECT_EQ(0, ecdsa_destroy());
}

TEST_F(TestEcdsa256, key)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";

	ecdsa_init();
	ecdsa_gen_keypair(public_key[0], private_key[0]);
	EXPECT_EQ(0, ecdsa_sign(message, sizeof(message), signature[0]));
	printf("signature: \n");
	print_array(signature[0], CONFIG_ECDSA_SIG_LEN);
	printf("----------------------------------------\n");

	ecdsa_gen_keypair(public_key[1], private_key[1]);
	EXPECT_EQ(0, ecdsa_verify(public_key[0], message, sizeof(message), signature[0]));

	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_ECDSA_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_ECDSA_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	ecdsa_destroy();
}

static float elapsed_time(struct timeval *end, struct timeval *start)
{
	float timeuse = 1000000 * (end->tv_sec - start->tv_sec) +
							  (end->tv_usec - start->tv_usec);
	return timeuse;
}

TEST_F(TestEcdsa256, time)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";

	struct timeval tpstart, tpend;

	gettimeofday(&tpstart, NULL);
	ecdsa_init();
	gettimeofday(&tpend, NULL);
	printf("ecdsa_init()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	ecdsa_gen_keypair(public_key[0], private_key[0]);
	gettimeofday(&tpend, NULL);
	printf("ecdsa_gen_keypair()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	ecdsa_sign(message, sizeof(message), signature[0]);
	gettimeofday(&tpend, NULL);
	printf("ecdsa_sign()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	ecdsa_gen_keypair(public_key[1], private_key[1]);

	gettimeofday(&tpstart, NULL);
	ecdsa_verify(public_key[0], message, sizeof(message), signature[0]);
	gettimeofday(&tpend, NULL);
	printf("ecdsa_verify()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	ecdsa_destroy();
	gettimeofday(&tpend, NULL);
	printf("ecdsa_destroy()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));
}
