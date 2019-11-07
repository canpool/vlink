/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>

#include "vos.h"

int v_random(void *output, int len)
{
    int i;
    int random_number;
    char *pbuf;

    if (output == NULL) {
        return -1;
    }

    pbuf = output;
    srand(vos_sys_time());

    for (i = 0; i < len; i += sizeof(int)) {
        random_number = rand();
        memcpy(pbuf + i, &random_number, sizeof(int) > len - i ? len - i : sizeof(int));
    }

    return 0;
}