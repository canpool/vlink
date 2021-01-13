/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vmodule.h"
#include "vlink.h"

int main(int argc, char **argv)
{
    int ret;

    vmodule_init();
    ret = vlink_main(argc, argv);
    vmodule_exit();

    return ret;
}
