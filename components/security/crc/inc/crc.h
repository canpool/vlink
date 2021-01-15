/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CRC_TEST_DATA   "vlink"
/**
 *  +=======================+=======================+
 *  | algorithm             | crc                   |
 *  +=======================+=======================+
 *  | crc8                  | 0xb9                  |
 *  | crc16                 | 0x0869                |
 *  | crc16_modbus          | 0x084d                |
 *  | crc_ccitt             | 0x27fd                |
 *  | crc_xmodem            | 0x36f1                |
 *  | crc32                 | 0xa758a49f            |
 *  | crc64_ecma            | 0x0f9fe7b75ce307dc    |
 *  | crc64_we              | 0xa39825c0a7a8200f    |
 *  +-----------------------+-----------------------+
 */

/* crc-8 */
uint8_t crc8(const uint8_t *data, size_t len);
uint8_t crc8_update(uint8_t crc, uint8_t ch);

/* crc-16 */
uint16_t crc16(const uint8_t *data, size_t len);
uint16_t crc16_modbus(const uint8_t *data, size_t len);
uint16_t crc16_update(uint16_t crc, uint8_t ch);

/* crc-ccitt (16 bit) */
uint16_t crc_ccitt(const uint8_t *data, size_t len);
uint16_t crc_xmodem(const uint8_t *data, size_t len);
uint16_t crc_ccitt_update(uint16_t crc, uint8_t ch);

/* crc-32 */
uint32_t crc32(const uint8_t *data, size_t len);
uint32_t crc32_update(uint16_t crc, uint8_t ch);

/* crc-64 */
uint64_t crc64_ecma(const uint8_t *data, size_t len);
uint64_t crc64_we(const uint8_t *data, size_t len);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CRC_H__ */
