/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "securec.h"

class TestSecurec : public ::testing::Test
{
protected:
    void SetUp() {
	};

	void TearDown() {
	};
};

TEST_F(TestSecurec, memset_s)
{
    char *p = (char *)malloc(10);
    ASSERT_TRUE(p != NULL);

    EXPECT_EQ(EINVAL, memset_s(NULL, 10, 0, 10));
    EXPECT_EQ(ERANGE, memset_s(p, 0, 0, 10));
    EXPECT_EQ(ERANGE_AND_RESET, memset_s(p, 10, 0, 11));
    EXPECT_EQ(EOK, memset_s(p, 10, 0, 10));

    free(p);
}

TEST_F(TestSecurec, memcpy_s)
{
    char src[] = {1, 2, 3, 4};
    char dst[sizeof(src) + 1] = {0};

    EXPECT_EQ(ERANGE, memcpy_s(NULL, 0, NULL, 0));
    EXPECT_EQ(ERANGE, memcpy_s(NULL, 0, src, 0));
    EXPECT_EQ(ERANGE, memcpy_s(NULL, 0, src, sizeof(src)));
    EXPECT_EQ(EINVAL, memcpy_s(NULL, 1, NULL, 0));
    EXPECT_EQ(EINVAL, memcpy_s(NULL, 1, src, 0));
    EXPECT_EQ(EINVAL, memcpy_s(NULL, 1, src, sizeof(src)));
    EXPECT_EQ(EINVAL_AND_RESET, memcpy_s(dst, 1, NULL, 0));
    EXPECT_EQ(EOK, memcpy_s(dst, 1, src, 0));
    EXPECT_EQ(ERANGE_AND_RESET, memcpy_s(dst, 1, src, sizeof(src)));
    EXPECT_EQ(EOK, memcpy_s(dst, sizeof(dst), src, sizeof(src)));
    EXPECT_EQ(0, memcmp(dst, src, sizeof(src)));

    // memory equal
    EXPECT_EQ(EOK, memcpy_s(src, sizeof(src), src, sizeof(src)));

    // memory overlap
    EXPECT_EQ(EOVERLAP_AND_RESET, memcpy_s(dst, sizeof(dst), dst + 1, sizeof(dst) - 1));
    EXPECT_EQ(EOVERLAP_AND_RESET, memcpy_s(dst + 1, sizeof(dst) - 1, dst, sizeof(dst) - 1));
}

TEST_F(TestSecurec, memmove_s)
{
    char src[] = {1, 2, 3, 4};
    char dst[sizeof(src) + 1] = {0};

    EXPECT_EQ(ERANGE, memmove_s(NULL, 0, NULL, 0));
    EXPECT_EQ(ERANGE, memmove_s(NULL, 0, src, 0));
    EXPECT_EQ(ERANGE, memmove_s(NULL, 0, src, sizeof(src)));
    EXPECT_EQ(EINVAL, memmove_s(NULL, 1, NULL, 0));
    EXPECT_EQ(EINVAL, memmove_s(NULL, 1, src, 0));
    EXPECT_EQ(EINVAL, memmove_s(NULL, 1, src, sizeof(src)));
    EXPECT_EQ(EINVAL_AND_RESET, memmove_s(dst, 1, NULL, 0));
    EXPECT_EQ(EOK, memmove_s(dst, 1, src, 0));
    EXPECT_EQ(ERANGE_AND_RESET, memmove_s(dst, 1, src, sizeof(src)));
    EXPECT_EQ(EOK, memmove_s(dst, sizeof(dst), src, sizeof(src)));
    EXPECT_EQ(0, memcmp(dst, src, sizeof(src)));

    // memory equal
    EXPECT_EQ(EOK, memmove_s(src, sizeof(src), src, sizeof(src)));

    // memory overlap
    EXPECT_EQ(EOK, memmove_s(dst, sizeof(dst), dst + 1, sizeof(dst) - 1));
    EXPECT_EQ(EOK, memmove_s(dst + 1, sizeof(dst) - 1, dst, sizeof(dst) - 1));
}

TEST_F(TestSecurec, strcpy_s)
{
    char src[] = "hello";
    char dst[sizeof(src)] = {0};

    EXPECT_EQ(ERANGE, strcpy_s(NULL, 0, NULL));
    EXPECT_EQ(ERANGE, strcpy_s(NULL, 0, src));
    EXPECT_EQ(EINVAL, strcpy_s(NULL, 1, NULL));
    EXPECT_EQ(EINVAL, strcpy_s(NULL, 1, src));
    EXPECT_EQ(EINVAL_AND_RESET, strcpy_s(dst, 1, NULL));
    EXPECT_EQ(ERANGE_AND_RESET, strcpy_s(dst, 1, src));
    EXPECT_EQ(EOK, strcpy_s(dst, sizeof(dst), src));
    EXPECT_EQ(0, strcmp(dst, "hello"));

    // memory equal
    EXPECT_EQ(EOK, strcpy_s(src, sizeof(src), src));

    // memory overlap
    EXPECT_EQ(EOVERLAP_AND_RESET, strcpy_s(dst, sizeof(dst), dst + 1));
    EXPECT_EQ(0, strlen(dst));
    EXPECT_EQ(EOK, strcpy_s(dst, sizeof(dst), src));
    EXPECT_EQ(ERANGE_AND_RESET, strcpy_s(dst + 1, sizeof(dst) - 1, dst));
    EXPECT_EQ(0, strcmp(dst, "h"));
}

TEST_F(TestSecurec, strncpy_s)
{
    char src[] = "hello";
    char dst[sizeof(src)] = {0};

    EXPECT_EQ(ERANGE, strncpy_s(NULL, 0, NULL, 0));
    EXPECT_EQ(ERANGE, strncpy_s(NULL, 0, src, 0));
    EXPECT_EQ(ERANGE, strncpy_s(NULL, 0, src, sizeof(src)));
    EXPECT_EQ(EINVAL, strncpy_s(NULL, 1, NULL, 0));
    EXPECT_EQ(EINVAL, strncpy_s(NULL, 1, src, 0));
    EXPECT_EQ(EINVAL, strncpy_s(NULL, 1, src, sizeof(src)));
    EXPECT_EQ(EINVAL_AND_RESET, strncpy_s(dst, 1, NULL, 0));
    EXPECT_EQ(EOK, strncpy_s(dst, 1, src, 0));
    EXPECT_EQ(ERANGE_AND_RESET, strncpy_s(dst, 1, src, sizeof(src)));
    EXPECT_EQ(EOK, strncpy_s(dst, sizeof(dst), src, sizeof(src)));
    EXPECT_EQ(0, strcmp(dst, "hello"));

    // memory equal
    EXPECT_EQ(EOK, strncpy_s(src, sizeof(src), src, sizeof(src)));

    // memory overlap
    EXPECT_EQ(EOVERLAP_AND_RESET, strncpy_s(dst, sizeof(dst), dst + 1, sizeof(dst) - 1));
    EXPECT_EQ(0, strlen(dst));
    EXPECT_EQ(EOK, strcpy_s(dst, sizeof(dst), src));
    EXPECT_EQ(EOVERLAP_AND_RESET, strncpy_s(dst + 1, sizeof(dst) - 1, dst, sizeof(dst) - 2));
    EXPECT_EQ(0, strcmp(dst, "h"));
}

TEST_F(TestSecurec, strcat_s)
{
    char src[] = "hello";
    char dst[sizeof(src) << 1] = {0};

    EXPECT_EQ(ERANGE, strcat_s(NULL, 0, NULL));
    EXPECT_EQ(ERANGE, strcat_s(NULL, 0, src));
    EXPECT_EQ(EINVAL, strcat_s(NULL, 1, NULL));
    EXPECT_EQ(EINVAL, strcat_s(NULL, 1, src));
    EXPECT_EQ(EINVAL_AND_RESET, strcat_s(dst, 1, NULL));
    EXPECT_EQ(ERANGE_AND_RESET, strcat_s(dst, 1, src));
    EXPECT_EQ(EOK, strcat_s(dst, sizeof(dst), src));
    EXPECT_EQ(0, strcmp(dst, "hello"));

    // memory equal
    EXPECT_EQ(ERANGE_AND_RESET, strcat_s(dst, sizeof(dst), dst));

    EXPECT_EQ(EOK, strcat_s(dst, sizeof(dst), src));

    // memory overlap
    EXPECT_EQ(EOVERLAP_AND_RESET, strcat_s(dst, sizeof(dst), dst + 1));
    EXPECT_EQ(0, strlen(dst));
    EXPECT_EQ(EOK, strcat_s(dst, sizeof(dst), src));
    EXPECT_EQ(EOVERLAP_AND_RESET, strcat_s(dst + 1, sizeof(dst) - 1, dst));
    EXPECT_EQ(0, strcmp(dst, "h"));

    EXPECT_EQ(EOK, strcat_s(dst, sizeof(dst), src));
    EXPECT_EQ(0, strcmp(dst, "hhello"));
}

TEST_F(TestSecurec, strncat_s)
{
    char src[] = "hello";
    char dst[sizeof(src) << 1] = {0};

    EXPECT_EQ(ERANGE, strncat_s(NULL, 0, NULL, 0));
    EXPECT_EQ(ERANGE, strncat_s(NULL, 0, src, 0));
    EXPECT_EQ(ERANGE, strncat_s(NULL, 0, src, strlen(src)));
    EXPECT_EQ(EINVAL, strncat_s(NULL, 1, NULL, 0));
    EXPECT_EQ(EINVAL, strncat_s(NULL, 1, src, 0));
    EXPECT_EQ(EINVAL, strncat_s(NULL, 1, src, strlen(src)));
    EXPECT_EQ(EINVAL_AND_RESET, strncat_s(dst, 1, NULL, 0));
    EXPECT_EQ(EOK, strncat_s(dst, 1, src, 0));
    EXPECT_EQ(ERANGE_AND_RESET, strncat_s(dst, 1, src, strlen(src)));
    EXPECT_EQ(EOK, strncat_s(dst, sizeof(dst), src, strlen(src)));
    EXPECT_EQ(0, strcmp(dst, "hello"));

    // memory equal
    EXPECT_EQ(ERANGE_AND_RESET, strncat_s(dst, sizeof(dst), dst, strlen(dst)));

    EXPECT_EQ(EOK, strncat_s(dst, sizeof(dst), src, strlen(src)));

    // memory overlap
    EXPECT_EQ(EOVERLAP_AND_RESET, strncat_s(dst, sizeof(dst), dst + 1, strlen(dst) - 1));
    EXPECT_EQ(0, strlen(dst));
    EXPECT_EQ(EOK, strcpy_s(dst, sizeof(dst), src));
    EXPECT_EQ(EOVERLAP_AND_RESET, strncat_s(dst + 1, sizeof(dst) - 1, dst, strlen(dst) - 2));
    EXPECT_EQ(0, strcmp(dst, "h"));

    EXPECT_EQ(EOK, strncat_s(dst, sizeof(dst), src, strlen(src)));
    EXPECT_EQ(0, strcmp(dst, "hhello"));
}

TEST_F(TestSecurec, sprintf_s)
{
    char dst[10] = {0};

    EXPECT_EQ(-1, sprintf_s(NULL, 0, NULL));
    EXPECT_EQ(-1, sprintf_s(NULL, 0, "hello"));
    EXPECT_EQ(-1, sprintf_s(NULL, 1, NULL));
    EXPECT_EQ(-1, sprintf_s(NULL, 1, "hello"));
    EXPECT_EQ(-1, sprintf_s(dst, 0, NULL));
    EXPECT_EQ(-1, sprintf_s(dst, 0, "hello"));
    EXPECT_EQ(-1, sprintf_s(dst, 1, NULL));
    EXPECT_EQ(-1, sprintf_s(dst, 1, "hello"));

    EXPECT_EQ(-1, sprintf_s(dst, 5, "hello"));
    EXPECT_EQ(5, sprintf_s(dst, 6, "hello"));

    EXPECT_EQ(5, sprintf_s(dst, 6, "%s", "hello"));

    EXPECT_EQ(-1, sprintf_s(dst, 6, "%s\n", "hello"));
    EXPECT_EQ(6, sprintf_s(dst, 7, "%s\n", "hello"));
}

TEST_F(TestSecurec, snprintf_s)
{
    char dst[10] = {0};

    EXPECT_EQ(-1, snprintf_s(NULL, 0, 0, NULL));
    EXPECT_EQ(-1, snprintf_s(NULL, 0, 0, "hello"));
    EXPECT_EQ(-1, snprintf_s(NULL, 1, 0, NULL));
    EXPECT_EQ(-1, snprintf_s(NULL, 1, 0, "hello"));
    EXPECT_EQ(-1, snprintf_s(dst, 0, 0, NULL));
    EXPECT_EQ(-1, snprintf_s(dst, 0, 0, "hello"));
    EXPECT_EQ(-1, snprintf_s(dst, 1, 0, NULL));
    EXPECT_EQ(-1, snprintf_s(dst, 1, 0, "hello"));

    EXPECT_EQ(-1, snprintf_s(dst, 5, 4, "hello"));
    EXPECT_EQ(5, snprintf_s(dst, 6, 5, "hello"));
    EXPECT_EQ(5, snprintf_s(dst, 6, 6, "hello"));

    EXPECT_EQ(5, snprintf_s(dst, 6, 5, "%s", "hello"));

    EXPECT_EQ(-1, snprintf_s(dst, 6, 5, "%s\n", "hello"));
    EXPECT_EQ(-1, snprintf_s(dst, 6, 6, "%s\n", "hello"));
    EXPECT_EQ(6, snprintf_s(dst, 7, 6, "%s\n", "hello"));
}
