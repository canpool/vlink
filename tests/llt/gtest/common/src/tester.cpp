/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "tester.h"
#include "gtest/gtest.h"

#include "vconfig.h"
#include "vos.h"

int tester_main(int argc, char **argv)
{
    int ret;

    testing::InitGoogleTest(&argc, argv);

#ifndef CONFIG_VMODULE
    vos_init();
#endif
    ret = RUN_ALL_TESTS();
#ifndef CONFIG_VMODULE
    vos_init();
#endif

    return ret;
}