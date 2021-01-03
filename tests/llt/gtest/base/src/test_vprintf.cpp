/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vprintf.h"

#include <cstdio>
#include <cstring>

extern "C" {

static int __uprintf_putc(uintptr_t arg, unsigned char ch)
{
    vputc(ch);

    if (ch == '\n') {
        vputc('\r');
    }

    return 1;
}

int uprintf(const char *format, ...)
{
    va_list valist;
    int     nbytes;

    va_start(valist, format);
    nbytes = xprintf(format, valist, __uprintf_putc, 0);
    va_end(valist);

    return nbytes;
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

TEST_F(TestPrintf, vprint_buffer)
{
    uint8_t buffer[] = {
        1, 2, 3, 4, 43, 32, 1, 1, 144, 88, 4, 6, 7, 8,
        9, 12, 13, 14, 15, 16, 16, 235, 7, 8, 99, 54
    };

    vprint_buffer(stdout, buffer, sizeof(buffer), 0);
}
