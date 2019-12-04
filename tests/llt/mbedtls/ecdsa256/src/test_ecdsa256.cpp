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

#include "vsl_ecdsa256.h"

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

extern "C" {
extern int vsl_ecdsa_init(void);
extern int vsl_ecdsa_destroy(void);
}

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

TEST_F(TestEcdsa256, init)
{
	EXPECT_EQ(0, vsl_ecdsa_init());
	EXPECT_EQ(0, vsl_ecdsa_destroy());
}

TEST_F(TestEcdsa256, key)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";
	unsigned char shared_key[2][CONFIG_ECDSA_SHRKEY_LEN] = {0};

	// vsl_ecdsa_init();
	vsl_ecdsa_gen_keypair(public_key[0], private_key[0]);
	EXPECT_EQ(0, vsl_ecdsa_sign(private_key[0], message, sizeof(message), signature[0]));
	printf("signature: \n");
	print_array(signature[0], CONFIG_ECDSA_SIG_LEN);
	printf("----------------------------------------\n");

	vsl_ecdsa_gen_keypair(public_key[1], private_key[1]);
	EXPECT_EQ(0, vsl_ecdsa_verify(public_key[0], message, sizeof(message), signature[0]));

	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_ECDSA_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_ECDSA_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	vsl_ecdsa_gen_shared(private_key[0], public_key[1], shared_key[0]);
	vsl_ecdsa_gen_shared(private_key[1], public_key[0], shared_key[1]);
	for (i = 0; i < 2; i++) {
		printf("shared key %d:\n", i);
		print_array(shared_key[i], CONFIG_ECDSA_SHRKEY_LEN);
	}
	for (i = 0; i < CONFIG_ECDSA_SHRKEY_LEN; i++) {
		EXPECT_EQ(shared_key[0][i], shared_key[1][i]);
	}

	// vsl_ecdsa_destroy();
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
	vsl_ecdsa_init();
	gettimeofday(&tpend, NULL);
	printf("vsl_ecdsa_init()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_ecdsa_destroy();
	gettimeofday(&tpend, NULL);
	printf("vsl_ecdsa_destroy()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_ecdsa_gen_keypair(public_key[0], private_key[0]);
	gettimeofday(&tpend, NULL);
	printf("vsl_ecdsa_gen_keypair()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	vsl_ecdsa_sign(private_key[0], message, sizeof(message), signature[0]);
	gettimeofday(&tpend, NULL);
	printf("vsl_ecdsa_sign()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	vsl_ecdsa_gen_keypair(public_key[1], private_key[1]);

	gettimeofday(&tpstart, NULL);
	vsl_ecdsa_verify(public_key[0], message, sizeof(message), signature[0]);
	gettimeofday(&tpend, NULL);
	printf("vsl_ecdsa_verify()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

}

TEST_F(TestEcdsa256, curve)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";
	unsigned char shared_key[2][CONFIG_ECDSA_SHRKEY_LEN] = {0};

	vsl_ecdsa_gen_keypair(public_key[0], private_key[0]);
	EXPECT_EQ(0, vsl_ecdsa_sign(private_key[0], message, sizeof(message), signature[0]));
	printf("signature: \n");
	print_array(signature[0], CONFIG_ECDSA_SIG_LEN);
	printf("----------------------------------------\n");

	vsl_ecdsa_set_curve(VSL_BP256R1_NAME);
	vsl_ecdsa_gen_keypair(public_key[1], private_key[1]);
	EXPECT_EQ(-1, vsl_ecdsa_verify(public_key[0], message, sizeof(message), signature[0]));

	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_ECDSA_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_ECDSA_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	vsl_ecdsa_gen_shared(private_key[0], public_key[1], shared_key[0]);
	vsl_ecdsa_gen_shared(private_key[1], public_key[0], shared_key[1]);
	for (i = 0; i < 2; i++) {
		printf("shared key %d:\n", i);
		print_array(shared_key[i], CONFIG_ECDSA_SHRKEY_LEN);
	}
}

#ifdef CONFIG_ECDSA256_CSR_MBEDTLS
TEST_F(TestEcdsa256, csr)
{
	const char *subject_name = "CN=Cert,serialNumber=123456,O=mbed TLS,C=UK";
	unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN] = {0};
	char buf[1024] = {0};
	unsigned int len = sizeof(buf);

	// note:
	// CSR generated with "secp256k1" or "secp256r1" can be verified with "prime256v1"
	// reference to: https://csr.chinassl.net/generator-csr.html
	vsl_ecdsa_set_curve(VSL_SECP256R1_NAME);

	vsl_ecdsa_gen_keypair(public_key, private_key);
	vsl_ecdsa_gen_csr(private_key, subject_name, buf, &len);
	printf("csr is:\n%s\nlen: %u\n", buf, len);
}
#endif

TEST_F(TestEcdsa256, frp256v1)
{
	unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN] = {0};

	vsl_curve_point frp256v1_points = {
		.p = {
			0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
			0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
			0x39, 0x61, 0xad, 0xbc, 0xab, 0xc8, 0xca, 0x6d,
			0xe8, 0xfc, 0xf3, 0x53, 0xd8, 0x6e, 0x9c, 0x03
		},
		.a = {
			0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
			0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
			0x39, 0x61, 0xad, 0xbc, 0xab, 0xc8, 0xca, 0x6d,
			0xe8, 0xfc, 0xf3, 0x53, 0xd8, 0x6e, 0x9c, 0x00
		},
		.b = {
			0xee, 0x35, 0x3f, 0xca, 0x54, 0x28, 0xa9, 0x30,
			0x0d, 0x4a, 0xba, 0x75, 0x4a, 0x44, 0xc0, 0x0f,
			0xdf, 0xec, 0x0c, 0x9a, 0xe4, 0xb1, 0xa1, 0x80,
			0x30, 0x75, 0xed, 0x96, 0x7b, 0x7b, 0xb7, 0x3f
		},
		.gx = {
			0xb6, 0xb3, 0xd4, 0xc3, 0x56, 0xc1, 0x39, 0xeb,
			0x31, 0x18, 0x3d, 0x47, 0x49, 0xd4, 0x23, 0x95,
			0x8c, 0x27, 0xd2, 0xdc, 0xaf, 0x98, 0xb7, 0x01,
			0x64, 0xc9, 0x7a, 0x2d, 0xd9, 0x8f, 0x5c, 0xff
		},
		.gy = {
			0x61, 0x42, 0xe0, 0xf7, 0xc8, 0xb2, 0x04, 0x91,
			0x1f, 0x92, 0x71, 0xf0, 0xf3, 0xec, 0xef, 0x8c,
			0x27, 0x01, 0xc3, 0x07, 0xe8, 0xe4, 0xc9, 0xe1,
			0x83, 0x11, 0x5a, 0x15, 0x54, 0x06, 0x2c, 0xfb
		},
		.n = {
			0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
			0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
			0x53, 0xdc, 0x67, 0xe1, 0x40, 0xd2, 0xbf, 0x94,
			0x1f, 0xfd, 0xd4, 0x59, 0xc6, 0xd6, 0x55, 0xe1
		}
	};

	vsl_ecdsa_def_curve(VSL_FRP256V1_NAME, &frp256v1_points);

	vsl_ecdsa_gen_keypair(public_key, private_key);

#ifdef CONFIG_ECDSA256_CSR_MBEDTLS
	const char *subject_name = "CN=Cert,serialNumber=123456,O=mbed TLS,C=UK";
	char buf[1024] = {0};
	unsigned int len = sizeof(buf);
	vsl_ecdsa_gen_csr(private_key, subject_name, buf, &len);
	printf("csr is:\n%s\nlen: %u\n", buf, len);
#endif

	vsl_curve_point prime256v1_points = {0};
	char name[CONFIG_CURVE_NAME_LEN] = {0};
	vsl_ecdsa_set_curve(VSL_PRIME256V1_NAME);
	vsl_ecdsa_get_curve(name, sizeof(name), &prime256v1_points);
	printf("curve name: %s\n", name);
	printf("p:\n");
	print_array(prime256v1_points.p, CONFIG_CURVE_POINT_LEN);
	printf("a:\n");
	print_array(prime256v1_points.a, CONFIG_CURVE_POINT_LEN);
	printf("b:\n");
	print_array(prime256v1_points.b, CONFIG_CURVE_POINT_LEN);
	printf("gx:\n");
	print_array(prime256v1_points.gx, CONFIG_CURVE_POINT_LEN);
	printf("gy:\n");
	print_array(prime256v1_points.gy, CONFIG_CURVE_POINT_LEN);
	printf("n:\n");
	print_array(prime256v1_points.n, CONFIG_CURVE_POINT_LEN);
}