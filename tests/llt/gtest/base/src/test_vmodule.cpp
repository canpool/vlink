/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "gtest/gtest.h"
#include "vmodule.h"

extern "C" {

int test_module_init(void)
{
    printf("test_module_init\n");
    return -1;
}

int test_module_exit(void)
{
    printf("test_module_exit\n");
    return 0;
}

VMODULE_DEF(user, test_module_init, test_module_exit);

}
