/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vos.h"

class TestVos : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestVos, memory)
{
    int size = 4;
    char *p = NULL;

    EXPECT_TRUE(vmem_malloc(0) == NULL);

    while (p = (char *)vmem_malloc(size)) {
        EXPECT_NO_THROW(vmem_free(p));
        if (size < (1 << 10)) {
            printf("%dB_", size);
        } else if (size < (1 << 20)) {
            printf("%dK_", size >> 10);
        } else if (size < (1 << 30)) {
            printf("%dM_", size >> 20);
        }
        size <<= 1;
    }
    printf("\n");

    size = 16;
    ASSERT_TRUE((p = (char *)vmem_malloc(size)) != NULL);
    for (int i = 0; i < size; ++i) {
        printf("%02x_", *(p + i));
    }
    EXPECT_NO_THROW(vmem_free(p));
    printf("\n");

    size = 4;
    ASSERT_TRUE((p = (char *)vmem_zalloc(size)) != NULL);
    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(0, *(p + i));
    }
    EXPECT_NO_THROW(vmem_free(p));

    size = 4;
    ASSERT_TRUE((p = (char *)vmem_calloc(1, size)) != NULL);
    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(0, *(p + i));
    }
    EXPECT_NO_THROW(vmem_free(p));

    ASSERT_TRUE((p = (char *)vmem_malloc(16)) != NULL);
    // realloc
    ASSERT_TRUE((p = (char *)vmem_realloc(p, 8)) != NULL);
    // free
    ASSERT_TRUE((p = (char *)vmem_realloc(p, 0)) == NULL);
    // new
    ASSERT_TRUE((p = (char *)vmem_realloc(NULL, 8)) != NULL);

    EXPECT_NO_THROW(vmem_free(p));

    ASSERT_TRUE((p = (char *)vmem_malloc(4)) != NULL);
    EXPECT_NO_THROW(vmem_sfree(p));
    EXPECT_TRUE(p == NULL);

    EXPECT_NO_THROW(vmem_stat());
}

TEST_F(TestVos, time)
{
    uint64_t start = mseconds();
    EXPECT_NO_THROW(msleep(5));
    uint64_t end = mseconds();

    EXPECT_TRUE(end - start >= 5);
    printf("%lu\n", end - start);
}
