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

#include "vsl_curve25519.h"

#include <sys/time.h>

#include "gtest/gtest.h"

class TestCurve25519 : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

extern "C" {
extern int vsl_curve25519_init(void);
extern int vsl_curve25519_destroy(void);
}

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

TEST_F(TestCurve25519, init)
{
	EXPECT_EQ(0, vsl_curve25519_init());
	EXPECT_EQ(0, vsl_curve25519_destroy());
}

TEST_F(TestCurve25519, key)
{
	int i;
	unsigned char public_key[2][CONFIG_CURVE_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_CURVE_PRIKEY_LEN] = {0};
	unsigned char shared_key[2][CONFIG_CURVE_SHRKEY_LEN] = {0};

	// vsl_curve25519_init();
	vsl_curve25519_gen_keypair(public_key[0], private_key[0]);
	vsl_curve25519_gen_keypair(public_key[1], private_key[1]);
	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_CURVE_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_CURVE_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	vsl_curve25519_gen_shared(private_key[0], public_key[1], shared_key[0]);
	vsl_curve25519_gen_shared(private_key[1], public_key[0], shared_key[1]);
	for (i = 0; i < 2; i++) {
		printf("shared key %d:\n", i);
		print_array(shared_key[i], CONFIG_CURVE_SHRKEY_LEN);
	}
	for (i = 0; i < CONFIG_CURVE_SHRKEY_LEN; i++) {
		EXPECT_EQ(shared_key[0][i], shared_key[1][i]);
	}

	// vsl_curve25519_destroy();
}

static float elapsed_time(struct timeval *end, struct timeval *start)
{
	float timeuse = 1000000 * (end->tv_sec - start->tv_sec) +
							  (end->tv_usec - start->tv_usec);
	return timeuse;
}

TEST_F(TestCurve25519, time)
{
	int i;
	unsigned char public_key[CONFIG_CURVE_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_CURVE_PRIKEY_LEN] = {0};
	unsigned char shared_key[CONFIG_CURVE_SHRKEY_LEN] = {0};

	struct timeval tpstart, tpend;

	gettimeofday(&tpstart, NULL);
	vsl_curve25519_init();
	gettimeofday(&tpend, NULL);
	printf("vsl_curve25519_init()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_curve25519_destroy();
	gettimeofday(&tpend, NULL);
	printf("vsl_curve25519_destroy()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_curve25519_gen_keypair(public_key, private_key);
	gettimeofday(&tpend, NULL);
	printf("vsl_curve25519_gen_keypair()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_curve25519_gen_shared(private_key, public_key, shared_key);
	gettimeofday(&tpend, NULL);
	printf("vsl_curve25519_gen_shared()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

}