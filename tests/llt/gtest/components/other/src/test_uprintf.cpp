/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "uprintf.h"
#include "vprintf.h"

#include <cstdio>
#include <cstring>

extern "C" {

static int __uprintf_putc(uintptr_t arg, unsigned char ch)
{
    uputc(ch);

    if (ch == '\n') {
        uputc('\r');
    }

    return 1;
}

int vuprintf(const char *format, va_list args)
{
    return xprintf(format, args, __uprintf_putc, 0);
}

}

class TestPrintf : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};



TEST_F(TestPrintf, uprintf)
{
    uprintf("%d\n", 1000000);
    uprintf("%ld\n", (uint64_t)1000000);
    uprintf("0x%-16llx %lld\n", 1000000, 1000000);
    uprintf("0x%016llx %lld\n", 1000000, 1000000);
    uprintf("0x%-16llx %lld\n", (unsigned long long)1000000, (unsigned long long)1000000);
    uprintf("0x%-16llx %lld\n", 1, 1);
    uprintf("0x%-16llx %lld\n", (unsigned long long)1, (unsigned long long)1);
    uprintf("%ld\n", (uint64_t)12345678901234);
    uprintf("%c\n", 'v');
    uprintf("%s\n", "vlink");
    uprintf("%.*s\n", 2, "vlink");
    uprintf("%p\n", (void *)NULL);
    uprintf("%%\n");
}

TEST_F(TestPrintf, printf)
{
    printf("%d\n", 1000000);
    printf("%ld\n", (uint64_t)1000000);
    printf("0x%-16llx %lld\n", 1000000, 1000000);
    printf("0x%016llx %lld\n", 1000000, 1000000);
    printf("0x%-16llx %lld\n", (unsigned long long)1000000, (unsigned long long)1000000);
    printf("0x%-16llx %lld\n", 1, 1);
    printf("0x%-16llx %lld\n", (unsigned long long)1, (unsigned long long)1);
    printf("%ld\n", (uint64_t)12345678901234);
    printf("%c\n", 'v');
    printf("%s\n", "vlink");
    printf("%.*s\n", 2, "vlink");
    printf("%p\n", (void *)NULL);
    printf("%%\n");
}
