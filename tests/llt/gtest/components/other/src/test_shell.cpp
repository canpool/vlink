/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vsh.h"

class TestShell : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

static unsigned char test_getchar(uintptr_t arg)
{
    unsigned char ch;

    while ((ch = (unsigned char)getchar()) == '\0')
        ;

    if (ch != 0x1b) {   // ESC = ^[
        return ch;
    }

    ch = (unsigned char)getchar();
    if (ch != 0x5b) {   // '['
        return ch;
    }

    ch = (unsigned char)getchar();
    switch (ch) {
    case 0x48:  // 'H'
        ch = (unsigned char)getchar(); /* eat 0x7e */
        return vsh_vk_home;
    case 0x33:  // '3'
        ch = (unsigned char)getchar(); /* eat 0x7e */
        return vsh_vk_del;
    case 0x46:  // 'F'
        ch = (unsigned char)getchar(); /* eat 0x7e */
        return vsh_vk_end;
    case 0x41:  // 'A'
        return vsh_vk_up;
    case 0x42:  // 'B'
        return vsh_vk_down;
    case 0x43:  // 'C'
        return vsh_vk_right;
    case 0x44:  // 'D'
        return vsh_vk_left;
    default:
        return '\0';
    }
}

static int test_putchar(uintptr_t arg, unsigned char ch)
{
    return putchar(ch);
}

static vsh_t s_shell = {
    "", 0, 0, 0, 0, 0, {0}, {0}, test_getchar, test_putchar, 0
};

#define TEST_VSH_HIS_CMD_BUF_SIZE       256
#define TEST_VSH_HIS_IDX_BUF_SIZE       64

TEST_F(TestShell, demo)
{
    static unsigned char his_content[TEST_VSH_HIS_CMD_BUF_SIZE];
    static unsigned char his_indexes[TEST_VSH_HIS_IDX_BUF_SIZE];

    vring_init(&s_shell.his_cmd, his_content, TEST_VSH_HIS_CMD_BUF_SIZE);
    vring_init(&s_shell.his_idx, his_indexes, TEST_VSH_HIS_IDX_BUF_SIZE);

    vsh_loop((uintptr_t)&s_shell);
}