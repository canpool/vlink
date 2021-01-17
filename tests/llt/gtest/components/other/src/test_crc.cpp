/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "crc.h"
#include "vconfig.h"

#include <cstdio>
#include <cstring>

extern "C" {
#ifdef CONFIG_CRC16
    extern void crc16_gen_tab(uint16_t table[256]);
#endif
#ifdef CONFIG_CRC_CCITT
    extern void crc_ccitt_gen_tab(uint16_t table[256]);
#endif
#ifdef CONFIG_CRC32
    extern void crc32_gen_tab(uint32_t table[256]);
#endif
#ifdef CONFIG_CRC64
    extern void crc64_gen_tab(uint64_t table[256]);
#endif
#ifdef CONFIG_CRC8
    extern void crc8_gen_tab(uint8_t table[256]);
#endif
}

static void print_crc8_table(uint8_t table[256])
{
    int i;

    for (i = 0; i < 256; i++) {
        printf("0x%02x ", table[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static void print_crc16_table(uint16_t table[256])
{
    int i;

    for (i = 0; i < 256; i++) {
        printf("0x%04x ", table[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static void print_crc32_table(uint32_t table[256])
{
    int i;

    for (i = 0; i < 256; i++) {
        printf("0x%08x ", table[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static void print_crc64_table(uint64_t table[256])
{
    int i;

    for (i = 0; i < 256; i++) {
        printf("0x%016llx ", table[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

class TestCRC : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

#ifdef CONFIG_CRC8
TEST_F(TestCRC, crc8)
{
    uint8_t table[256] = {0};
    crc8_gen_tab(table);
    printf("crc8_gen_tab:\n");
    print_crc8_table(table);

    uint8_t val = crc8((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc8 crc: 0x%02x\n", val);
}
#endif

#ifdef CONFIG_CRC16
TEST_F(TestCRC, crc16)
{
    uint16_t table[256] = {0};
    crc16_gen_tab(table);
    printf("crc16_gen_table:\n");
    print_crc16_table(table);

    uint16_t val = crc16((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc16 crc: 0x%04x\n", val);
    val = crc16_modbus((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc16_modbus crc: 0x%04x\n", val);
}
#endif

#ifdef CONFIG_CRC_CCITT
TEST_F(TestCRC, crc_ccitt)
{
    uint16_t table[256] = {0};
    crc_ccitt_gen_tab(table);
    printf("crc_ccitt_gen_tab:\n");
    print_crc16_table(table);

    uint16_t val = crc_ccitt((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc_ccitt crc: 0x%04x\n", val);
    val = crc_xmodem((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc_xmodem crc: 0x%04x\n", val);
}
#endif

#ifdef CONFIG_CRC32
TEST_F(TestCRC, crc32)
{
    uint32_t table[256] = {0};
    crc32_gen_tab(table);
    printf("crc32_gen_tab:\n");
    print_crc32_table(table);

    uint32_t val = crc32((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc32 crc: 0x%08x\n", val);
}
#endif

#ifdef CONFIG_CRC64
TEST_F(TestCRC, crc64)
{
    uint64_t table[256] = {0};
    crc64_gen_tab(table);
    printf("crc64_gen_tab:\n");
    print_crc64_table(table);

    uint64_t val = crc64_ecma((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc64_ecma crc: 0x%016llx\n", val);
    val = crc64_we((uint8_t *)CRC_TEST_DATA, strlen(CRC_TEST_DATA));
    printf("crc64_we crc: 0x%016llx\n", val);
}
#endif
