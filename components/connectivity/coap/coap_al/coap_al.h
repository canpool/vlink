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

#ifndef __COAP_AL_H__
#define __COAP_AL_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* message types */
#define COAP_AL_MESSAGE_CON       0 /* confirmable message (requires ACK/RST) */
#define COAP_AL_MESSAGE_NON       1 /* non-confirmable message (one-shot message) */
#define COAP_AL_MESSAGE_ACK       2 /* used to acknowledge confirmable messages */
#define COAP_AL_MESSAGE_RST       3 /* indicates error in received messages */

/* request methods */
#define COAP_AL_REQUEST_GET       1
#define COAP_AL_REQUEST_POST      2
#define COAP_AL_REQUEST_PUT       3
#define COAP_AL_REQUEST_DELETE    4

#define COAP_AL_OPTION_IF_MATCH        1 /* C, opaque, 0-8 B, (none) */
#define COAP_AL_OPTION_URI_HOST        3 /* C, String, 1-255 B, destination address */
#define COAP_AL_OPTION_ETAG            4 /* E, opaque, 1-8 B, (none) */
#define COAP_AL_OPTION_IF_NONE_MATCH   5 /* empty, 0 B, (none) */
#define COAP_AL_OPTION_URI_PORT        7 /* C, uint, 0-2 B, destination port */
#define COAP_AL_OPTION_LOCATION_PATH   8 /* E, String, 0-255 B, - */
#define COAP_AL_OPTION_URI_PATH       11 /* C, String, 0-255 B, (none) */
#define COAP_AL_OPTION_CONTENT_FORMAT 12 /* E, uint, 0-2 B, (none) */
#define COAP_AL_OPTION_CONTENT_TYPE   COAP_AL_OPTION_CONTENT_FORMAT
#define COAP_AL_OPTION_MAXAGE         14 /* E, uint, 0--4 B, 60 Seconds */
#define COAP_AL_OPTION_URI_QUERY      15 /* C, String, 1-255 B, (none) */
#define COAP_AL_OPTION_ACCEPT         17 /* C, uint,   0-2 B, (none) */
#define COAP_AL_OPTION_LOCATION_QUERY 20 /* E, String,   0-255 B, (none) */
#define COAP_AL_OPTION_PROXY_URI      35 /* C, String, 1-1034 B, (none) */
#define COAP_AL_OPTION_PROXY_SCHEME   39 /* C, String, 1-255 B, (none) */
#define COAP_AL_OPTION_SIZE1          60 /* E, uint, 0-4 B, (none) */

/* option types from RFC 7641 */
#define COAP_AL_OPTION_OBSERVE         6 /* E, empty/uint, 0 B/0-3 B, (none) */
#define COAP_AL_OPTION_SUBSCRIPTION  COAP_AL_OPTION_OBSERVE

/* selected option types from RFC 7959 */
#define COAP_AL_OPTION_BLOCK2         23 /* C, uint, 0--3 B, (none) */
#define COAP_AL_OPTION_BLOCK1         27 /* C, uint, 0--3 B, (none) */

/* selected option types from RFC 7967 */
#define COAP_AL_OPTION_NORESPONSE    258 /* N, uint, 0--1 B, 0 */

#define COAP_AL_MAX_OPT              65535 /**< the highest option number we know */

#define COAP_AL_RESP_CODE(N)         (((N)/100 << 5) | (N) % 100)

typedef int (*cmd_dealer_fn)(void *msg, int len);

typedef struct {
    char            *endpoint;
    char            *host;
    char            *port;

    uint32_t         keeplive;
    cmd_dealer_fn    dealer;

    // for dtls
    unsigned char   *psk_identity;
    size_t           psk_identity_len;
    unsigned char   *psk_key;
    size_t           psk_key_len;
} coap_al_config_t;

typedef uintptr_t   coaper_t;

typedef struct {
    int    (*init)      (uintptr_t *handle, coap_al_config_t *config);
    int    (*destroy)   (uintptr_t  handle);
    int    (*add_option)(uintptr_t  handle, uint16_t number, size_t len, const uint8_t *data);
    void * (*request)   (uintptr_t  handle, uint8_t msgtype, uint8_t code, uint8_t *payload, size_t len);
    int    (*send)      (uintptr_t  handle, void *msg);
    int    (*recv)      (uintptr_t  handle);
} coap_al_ops_t;

int     coap_al_init(coaper_t *coaper, coap_al_config_t *config);
int     coap_al_destroy(coaper_t coaper);
int     coap_al_add_option(coaper_t coaper, uint16_t number, size_t len, const uint8_t *data);
void *  coap_al_request(coaper_t coaper, uint8_t msgtype, uint8_t code, uint8_t *payload, size_t len);
int     coap_al_send(coaper_t coaper, void *msg);
int     coap_al_recv(coaper_t coaper);

int     coap_al_install(const coap_al_ops_t *ops);
int     coap_al_uninstall(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COAP_AL_H__ */