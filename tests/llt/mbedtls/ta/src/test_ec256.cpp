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

#include "ec256.h"

#include <sys/time.h>
#include <stdint.h>

#include "gtest/gtest.h"

#define CONFIG_ECDSA_PUBKEY_LEN 64 /* public key (X+Y) */
#define CONFIG_ECDSA_PRIKEY_LEN 32 /* private key */
#define CONFIG_ECDSA_SIG_LEN    64 /* raw signature (r+s), not encoded by ANS.1 */
#define CONFIG_ECDSA_SHRKEY_LEN 64 /* shared */

#define CONFIG_CURVE_NAME_LEN   20
#define CONFIG_CURVE_POINT_LEN  32

class TestEc256 : public ::testing::Test
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

TEST_F(TestEc256, key)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";
	unsigned char shared_key[2][CONFIG_ECDSA_SHRKEY_LEN] = {0};

	TA_ec256_create_key_pair(private_key[0], public_key[0]);
	EXPECT_EQ(0, TA_ec256_ecdsa_sign(message, sizeof(message), private_key[0], signature[0]));
	printf("signature: \n");
	print_array(signature[0], CONFIG_ECDSA_SIG_LEN);
	printf("----------------------------------------\n");

	TA_ec256_create_key_pair(private_key[1], public_key[1]);
	EXPECT_EQ(0, TA_ec256_ecdsa_verify(message, sizeof(message), signature[0], public_key[0]));

	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_ECDSA_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_ECDSA_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	TA_ec256_ecdh(private_key[0], public_key[1], shared_key[0]);
	TA_ec256_ecdh(private_key[1], public_key[0], shared_key[1]);
	for (i = 0; i < 2; i++) {
		printf("shared key %d:\n", i);
		print_array(shared_key[i], CONFIG_ECDSA_SHRKEY_LEN);
	}
	for (i = 0; i < CONFIG_ECDSA_SHRKEY_LEN; i++) {
		EXPECT_EQ(shared_key[0][i], shared_key[1][i]);
	}
}

static float elapsed_time(struct timeval *end, struct timeval *start)
{
	float timeuse = 1000000 * (end->tv_sec - start->tv_sec) +
							  (end->tv_usec - start->tv_usec);
	return timeuse;
}

TEST_F(TestEc256, time)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";

	struct timeval tpstart, tpend;

	gettimeofday(&tpstart, NULL);
	TA_ec256_create_key_pair(private_key[0], public_key[0]);
	gettimeofday(&tpend, NULL);
	printf("TA_ec256_create_key_pair()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	gettimeofday(&tpstart, NULL);
	TA_ec256_ecdsa_sign(message, sizeof(message), private_key[0], signature[0]);
	gettimeofday(&tpend, NULL);
	printf("TA_ec256_ecdsa_sign()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

	TA_ec256_create_key_pair(private_key[1], public_key[1]);

	gettimeofday(&tpstart, NULL);
	TA_ec256_ecdsa_verify(message, sizeof(message), signature[0], public_key[0]);
	gettimeofday(&tpend, NULL);
	printf("TA_ec256_ecdsa_verify()\n\telapsed time %f us\n", elapsed_time(&tpend, &tpstart));

}

TEST_F(TestEc256, curve)
{
	int i;
	unsigned char public_key[2][CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[2][CONFIG_ECDSA_PRIKEY_LEN] = {0};
	unsigned char signature[2][CONFIG_ECDSA_SIG_LEN] = {0};
	unsigned char message[20] = "hello ecdsa";
	unsigned char shared_key[2][CONFIG_ECDSA_SHRKEY_LEN] = {0};

	TA_ec256_create_key_pair(private_key[0], public_key[0]);
	EXPECT_EQ(0, TA_ec256_ecdsa_sign(message, sizeof(message), private_key[0], signature[0]));
	printf("signature: \n");
	print_array(signature[0], CONFIG_ECDSA_SIG_LEN);
	printf("----------------------------------------\n");

	TA_ec256_register_curve((const unsigned char *)TA_BP256R1_NAME);
	TA_ec256_create_key_pair(private_key[1], public_key[1]);
	EXPECT_NE(0, TA_ec256_ecdsa_verify(message, sizeof(message), signature[0], public_key[0]));

	for (i = 0; i < 2; i++) {
		printf("keypair %d:\n", i);
		printf("public key: \n");
		print_array(public_key[i], CONFIG_ECDSA_PUBKEY_LEN);
		printf("private key: \n");
		print_array(private_key[i], CONFIG_ECDSA_PRIKEY_LEN);
		printf("----------------------------------------\n");
	}

	TA_ec256_ecdh(private_key[0], public_key[1], shared_key[0]);
	TA_ec256_ecdh(private_key[1], public_key[0], shared_key[1]);
	for (i = 0; i < 2; i++) {
		printf("shared key %d:\n", i);
		print_array(shared_key[i], CONFIG_ECDSA_SHRKEY_LEN);
	}
}

TEST_F(TestEc256, csr)
{
	const char *subject_name = "CN=Cert,serialNumber=123456,O=mbed TLS,C=UK";
	unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN] = {0};
	char buf[1024] = {0};
	unsigned short len = (unsigned short)sizeof(buf);

	// note:
	// CSR generated with "secp256k1" or "secp256r1" can be verified with "prime256v1"
	// reference to: https://csr.chinassl.net/generator-csr.html
	TA_ec256_register_curve((const unsigned char *)TA_PRIME256V1_NAME);

	TA_ec256_create_key_pair(private_key, public_key);
	TA_ec256_create_csr(private_key, (const unsigned char *)subject_name,
		(unsigned char *)buf, &len);
	printf("csr is:\n%s\nlen: %u\n", buf, len);
}

TEST_F(TestEc256, frp256v1)
{
	unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN] = {0};

	uint8_t frp256v1_p[] = {
		0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
		0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
		0x39, 0x61, 0xad, 0xbc, 0xab, 0xc8, 0xca, 0x6d,
		0xe8, 0xfc, 0xf3, 0x53, 0xd8, 0x6e, 0x9c, 0x03
	};

	uint8_t frp256v1_a[] = {
		0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
		0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
		0x39, 0x61, 0xad, 0xbc, 0xab, 0xc8, 0xca, 0x6d,
		0xe8, 0xfc, 0xf3, 0x53, 0xd8, 0x6e, 0x9c, 0x00
	};

	uint8_t frp256v1_b[] = {
		0xee, 0x35, 0x3f, 0xca, 0x54, 0x28, 0xa9, 0x30,
		0x0d, 0x4a, 0xba, 0x75, 0x4a, 0x44, 0xc0, 0x0f,
		0xdf, 0xec, 0x0c, 0x9a, 0xe4, 0xb1, 0xa1, 0x80,
		0x30, 0x75, 0xed, 0x96, 0x7b, 0x7b, 0xb7, 0x3f
	};

	uint8_t frp256v1_gx[] = {
		0xb6, 0xb3, 0xd4, 0xc3, 0x56, 0xc1, 0x39, 0xeb,
		0x31, 0x18, 0x3d, 0x47, 0x49, 0xd4, 0x23, 0x95,
		0x8c, 0x27, 0xd2, 0xdc, 0xaf, 0x98, 0xb7, 0x01,
		0x64, 0xc9, 0x7a, 0x2d, 0xd9, 0x8f, 0x5c, 0xff
	};

	uint8_t frp256v1_gy[] = {
		0x61, 0x42, 0xe0, 0xf7, 0xc8, 0xb2, 0x04, 0x91,
		0x1f, 0x92, 0x71, 0xf0, 0xf3, 0xec, 0xef, 0x8c,
		0x27, 0x01, 0xc3, 0x07, 0xe8, 0xe4, 0xc9, 0xe1,
		0x83, 0x11, 0x5a, 0x15, 0x54, 0x06, 0x2c, 0xfb
	};

	uint8_t frp256v1_n[] = {
		0xf1, 0xfd, 0x17, 0x8c, 0x0b, 0x3a, 0xd5, 0x8f,
		0x10, 0x12, 0x6d, 0xe8, 0xce, 0x42, 0x43, 0x5b,
		0x53, 0xdc, 0x67, 0xe1, 0x40, 0xd2, 0xbf, 0x94,
		0x1f, 0xfd, 0xd4, 0x59, 0xc6, 0xd6, 0x55, 0xe1
	};

	TA_ec256_register_custom_curve((unsigned char *)TA_FRP256V1_NAME, frp256v1_a, frp256v1_b,
		frp256v1_p, frp256v1_gx, frp256v1_gy, frp256v1_n);

	TA_ec256_create_key_pair(private_key, public_key);


	const char *subject_name = "CN=Cert,serialNumber=123456,O=mbed TLS,C=UK";
	char buf[1024] = {0};
	unsigned short len = (unsigned short)sizeof(buf);
	TA_ec256_create_csr(private_key, (const unsigned char *)subject_name,
		(unsigned char *)buf, &len);
	printf("csr is:\n%s\nlen: %u\n", buf, len);


	uint8_t prime256v1_a[CONFIG_CURVE_POINT_LEN]  = {0};
	uint8_t prime256v1_b[CONFIG_CURVE_POINT_LEN]  = {0};
	uint8_t prime256v1_p[CONFIG_CURVE_POINT_LEN]  = {0};
	uint8_t prime256v1_gx[CONFIG_CURVE_POINT_LEN] = {0};
	uint8_t prime256v1_gy[CONFIG_CURVE_POINT_LEN] = {0};
	uint8_t prime256v1_n[CONFIG_CURVE_POINT_LEN]  = {0};
	unsigned char name[CONFIG_CURVE_NAME_LEN] = {0};
	TA_ec256_register_curve((const unsigned char *)TA_PRIME256V1_NAME);
	TA_ec256_get_curve(name, prime256v1_a, prime256v1_b, prime256v1_p,
		prime256v1_gx, prime256v1_gy, prime256v1_n);
	printf("curve name: %s\n", name);
	printf("p:\n");
	print_array(prime256v1_p, CONFIG_CURVE_POINT_LEN);
	printf("a:\n");
	print_array(prime256v1_a, CONFIG_CURVE_POINT_LEN);
	printf("b:\n");
	print_array(prime256v1_b, CONFIG_CURVE_POINT_LEN);
	printf("gx:\n");
	print_array(prime256v1_gx, CONFIG_CURVE_POINT_LEN);
	printf("gy:\n");
	print_array(prime256v1_gy, CONFIG_CURVE_POINT_LEN);
	printf("n:\n");
	print_array(prime256v1_n, CONFIG_CURVE_POINT_LEN);
}

TEST_F(TestEc256, md5)
{
	unsigned char *input = (unsigned char *)"hello md5";
	unsigned char output[16] = {0};

	TA_md5(input, strlen((char *)input), output);
	printf("md5:\n");
	print_array(output, 16);
}

TEST_F(TestEc256, sha256)
{
	unsigned char *input = (unsigned char *)"hello sha256";
	unsigned char output[32] = {0};

	TA_sha256(input, strlen((char *)input), output);
	printf("sha256:\n");
	print_array(output, 32);
}

TEST_F(TestEc256, on_curve)
{
	unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN] = {0};
	unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN] = {0};

	TA_ec256_create_key_pair(private_key, public_key);

	EXPECT_EQ(0, TA_ec256_is_on_curve(public_key));

	public_key[0] = 'a';
	public_key[1] = 'b';
	EXPECT_NE(0, TA_ec256_is_on_curve(public_key));
}