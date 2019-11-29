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

#include "coap_al.h"
#include "vos.h"

typedef struct {
    const coap_al_ops_t   *ops;
} coap_al_cb_t;

typedef struct {
    uintptr_t            magic;
    uintptr_t            handle;
} coap_al_context_t;

static coap_al_cb_t s_coap_al;

int coap_al_init(coaper_t *coaper, coap_al_config_t *config)
{
    coap_al_context_t *al_ctx = NULL;

    if (coaper == NULL || config == NULL) {
        return -1;
    }
    al_ctx = (coap_al_context_t *)(*coaper);
    if (magic_verify(al_ctx)) {
        return -1;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->init == NULL) {
        return -1;
    }

    al_ctx = (coap_al_context_t *)vos_zalloc(sizeof(coap_al_context_t));
    if (al_ctx == NULL) {
        return -1;
    }

    if (s_coap_al.ops->init(&al_ctx->handle, config) != 0) {
        vos_free(al_ctx);
        return -1;
    }
    al_ctx->magic = (uintptr_t)al_ctx;
    *coaper = (coaper_t)al_ctx;

    return 0;
}

int coap_al_destroy(coaper_t coaper)
{
    coap_al_context_t *al_ctx = (coap_al_context_t *)(coaper);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->destroy == NULL) {
        return -1;
    }
    if (s_coap_al.ops->destroy(al_ctx->handle) != 0) {
        return -1;
    }
    al_ctx->magic = 0;
    vos_free(al_ctx);

    return 0;
}

int coap_al_add_option(coaper_t coaper, uint16_t number, size_t len, const uint8_t *data)
{
    coap_al_context_t *al_ctx = (coap_al_context_t *)(coaper);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (data == NULL || len == 0) {
        return -1;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->add_option == NULL) {
        return -1;
    }
    return s_coap_al.ops->add_option(al_ctx->handle, number, len, data);
}

void * coap_al_request(coaper_t coaper, uint8_t msgtype, uint8_t code, uint8_t *payload, size_t len)
{
    coap_al_context_t *al_ctx = (coap_al_context_t *)(coaper);

    if (!magic_verify(al_ctx)) {
        return NULL;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->request == NULL) {
        return NULL;
    }
    return s_coap_al.ops->request(al_ctx->handle, msgtype, code, payload, len);
}

int coap_al_send(coaper_t coaper, void *msg)
{
    coap_al_context_t *al_ctx = (coap_al_context_t *)(coaper);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->send == NULL) {
        return -1;
    }
    return s_coap_al.ops->send(al_ctx->handle, msg);
}

int coap_al_recv(coaper_t coaper)
{
    coap_al_context_t *al_ctx = (coap_al_context_t *)(coaper);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_coap_al.ops == NULL || s_coap_al.ops->recv == NULL) {
        return -1;
    }
    return s_coap_al.ops->recv(al_ctx->handle);
}

int coap_al_install(const coap_al_ops_t *ops)
{
    if (ops == NULL || s_coap_al.ops != NULL) {
        return -1;
    }
    s_coap_al.ops = ops;
    return 0;
}

int coap_al_uninstall(void)
{
    if (s_coap_al.ops == NULL) {
        return -1;
    }
    s_coap_al.ops = NULL;
    return 0;
}