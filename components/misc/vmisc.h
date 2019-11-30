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

#ifndef __VMISC_H__
#define __VMISC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int v_random(void *output, int len);

char *v_strdup(const char *s);

// 0x12, 0xAB   => '1''2''A''B'
//              => '1''2''a''b'
int v_hexify(const unsigned char *in, unsigned int ilen, char *out, unsigned int olen, char lowercase);
// '1''2''A''B' => 0x12, 0xAB
int v_unhexify(const char *in, unsigned int ilen, unsigned char *out, unsigned int olen);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VMISC_H__ */