/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "tester.h"
#include "gtest/gtest.h"

int tester_main(int argc, char **argv)
{
    int ret;

    testing::InitGoogleTest(&argc, argv);

    ret = RUN_ALL_TESTS();

    return ret;
}