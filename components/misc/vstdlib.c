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

#include "vos.h"

// xdigit to xchar
#ifndef TOXCHAR // half-byte, islower
#define TOXCHAR(b, l)   ((b) > 9 ? ((b) - 10 + ((l) ? 'a' : 'A')) : ((b) + '0'))
#endif

// xchar to xdigit
#ifndef TOXDIGIT
#define TOXDIGIT(c)     (((c) <= '9') ? ((c) - 0x30) : (TOUPPER(c) - 0x37))
#endif

int v_hexify(const unsigned char *in, unsigned int ilen, char *out, unsigned int olen, char lowercase)
{
    if (in == NULL || ilen == 0 || out == NULL || olen < (ilen << 1)) {
        return -1;
    }
    int i;
    char high, low;

    for (i = 0; i < ilen; ++i) {
        high = (in[i] >> 4) & 0x0f;
        low  = (in[i])      & 0x0f;

        out[i << 1]         = TOXCHAR(high, lowercase);
        out[(i << 1) + 1]   = TOXCHAR(low,  lowercase);
    }
    return 0;
}

int v_unhexify(const char *in, unsigned int ilen, unsigned char *out, unsigned int olen)
{
    if (in == NULL || (ilen % 2 != 0) || out == NULL || (olen < ilen / 2)) {
        return -1;
    }
    int i;
    unsigned char high, low;

    for (i = 0; i < ilen; i += 2) {
        high = (unsigned char)in[i];
        low  = (unsigned char)in[i + 1];

        if (!ISXDIGIT(high) || !ISXDIGIT(low)) {
            return -1;
        }
        high = TOXDIGIT(high);
        low  = TOXDIGIT(low);

        out[i >> 1] = (high << 4) | (low & 0x0f);
    }
    return 0;
}