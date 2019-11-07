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

#ifndef __JFFS2_ZLIB_H__
#define __JFFS2_ZLIB_H__

#include <cyg/compress/zlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define zlib_deflateInit(x,y) deflateInit(x,y)
#define zlib_deflate(x,y) deflate(x,y)
#define zlib_deflateEnd(x) deflateEnd(x)
#define zlib_inflateInit(x) inflateInit(x)
#define zlib_inflateInit2(x,y) inflateInit2(x,y)
#define zlib_inflate(x,y) inflate(x,y)
#define zlib_inflateEnd(x) inflateEnd(x)

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_ZLIB_H__ */

