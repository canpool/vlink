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

#ifndef __OC_COAP_H__
#define __OC_COAP_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int (*oc_recv_fn)(void *msg, int len);

typedef struct {
    char            *endpoint;
    char            *host;
    char            *port;
    unsigned char   *psk;
    size_t           psk_len;
    unsigned char   *psk_id;
    size_t           psk_id_len;
} oc_server_t;

typedef struct {
    oc_server_t bs_server;
    oc_server_t app_server;
    oc_recv_fn  recv;
    void       *usr_data;
} oc_config_t;

typedef uintptr_t   oc_context_t;

typedef struct oc_ops {
    int (*init)     (uintptr_t *, oc_config_t *);
    int (*destroy)  (uintptr_t);
    int (*report)   (uintptr_t, char *, int);
} oc_ops_t;

int oc_coap_init(oc_context_t *ctx, oc_config_t *config);
int oc_coap_destroy(oc_context_t ctx);
int oc_coap_report(oc_context_t ctx, char *msg, int len);

int oc_coap_register(const char *name, const oc_ops_t *ops);
int oc_coap_unregister(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OC_COAP_H__ */