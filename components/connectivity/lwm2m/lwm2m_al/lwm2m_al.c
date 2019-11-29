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

#include "lwm2m_al.h"
#include "vos.h"

typedef struct {
    const lwm2m_al_ops_t    *ops;
} lwm2m_al_cb_t;

typedef struct {
    uintptr_t            magic;
    uintptr_t            handle;
} lwm2m_al_context_t;

static lwm2m_al_cb_t s_lwm2m_al;

int lwm2m_al_init(lwm2mer_t *m2m, lwm2m_al_config_t *config)
{
    lwm2m_al_context_t *al_ctx = NULL;

    if (m2m == NULL || config == NULL) {
        return -1;
    }
    al_ctx = (lwm2m_al_context_t *)(*m2m);
    if (magic_verify(al_ctx)) {
        return -1;
    }
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->init == NULL) {
        return -1;
    }

    al_ctx = (lwm2m_al_context_t *)vos_zalloc(sizeof(lwm2m_al_context_t));
    if (al_ctx == NULL) {
        return -1;
    }

    if (s_lwm2m_al.ops->init(&al_ctx->handle, config) != 0) {
        vos_free(al_ctx);
        return -1;
    }
    al_ctx->magic = (uintptr_t)al_ctx;
    *m2m = (lwm2mer_t)al_ctx;

    return 0;
}

int lwm2m_al_destroy(lwm2mer_t m2m)
{
    lwm2m_al_context_t *al_ctx = (lwm2m_al_context_t *)(m2m);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->destroy == NULL) {
        return -1;
    }
    if (s_lwm2m_al.ops->destroy(al_ctx->handle) != 0) {
        return -1;
    }
    al_ctx->magic = 0;
    vos_free(al_ctx);

    return 0;
}

int lwm2m_al_send(lwm2mer_t m2m, const char *uri, const char *msg, int len, uint32_t timeout)
{
    lwm2m_al_context_t *al_ctx = (lwm2m_al_context_t *)(m2m);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (uri == NULL || msg == NULL || len <= 0) {
        return -1;
    }
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->send == NULL) {
        return -1;
    }
    if (s_lwm2m_al.ops->send(al_ctx->handle, uri, msg, len, timeout) != 0) {
        return -1;
    }
    return 0;
}

int lwm2m_al_install(const lwm2m_al_ops_t *ops)
{
    if (ops == NULL || s_lwm2m_al.ops != NULL) {
        return -1;
    }
    s_lwm2m_al.ops = ops;
    return 0;
}

int lwm2m_al_uninstall(void)
{
    if (s_lwm2m_al.ops == NULL) {
        return -1;
    }
    s_lwm2m_al.ops = NULL;
    return 0;
}