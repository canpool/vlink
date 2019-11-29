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

#include "oc_lwm2m.h"
#include "vos.h"

#include <string.h>

typedef struct {
    const char      *name;
    const oc_ops_t  *ops;
} oc_lwm2m_t;

typedef struct {
    uintptr_t       magic;
    uintptr_t       handle;
    const oc_ops_t *ops;
} oc_lwm2m_context_t;

static oc_lwm2m_t s_lwm2m;

int oc_lwm2m_init(oc_context_t *ctx, oc_config_t *config)
{
    oc_lwm2m_context_t *oc_ctx = NULL;

    if (ctx == NULL || config == NULL) {
        return -1;
    }
    oc_ctx = (oc_lwm2m_context_t *)(*ctx);
    if (magic_verify(oc_ctx)) {
        return -1;
    }
    if (s_lwm2m.ops == NULL || s_lwm2m.ops->init == NULL) {
        return -1;
    }

    oc_ctx = (oc_lwm2m_context_t *)vos_zalloc(sizeof(oc_lwm2m_context_t));
    if (oc_ctx == NULL) {
        return -1;
    }
    if (s_lwm2m.ops->init(&oc_ctx->handle, config) != 0) {
        vos_free(oc_ctx);
        return -1;
    }
    oc_ctx->ops = s_lwm2m.ops;
    oc_ctx->magic = (uintptr_t)oc_ctx;
    *ctx = (oc_context_t)oc_ctx;

    return 0;
}

int oc_lwm2m_destroy(oc_context_t ctx)
{
    oc_lwm2m_context_t *oc_ctx = (oc_lwm2m_context_t *)ctx;

    if (!magic_verify(oc_ctx)) {
        return -1;
    }
    if (oc_ctx->ops == NULL || oc_ctx->ops->destroy == NULL) {
        return -1;
    }
    if (oc_ctx->ops->destroy(oc_ctx->handle) != 0) {
        return -1;
    }
    oc_ctx->magic = 0;
    vos_free(oc_ctx);

    return 0;
}

int oc_lwm2m_report(oc_context_t ctx, int type, char *msg, int len, uint32_t timeout)
{
    oc_lwm2m_context_t *oc_ctx = (oc_lwm2m_context_t *)ctx;

    if (!magic_verify(oc_ctx)) {
        return -1;
    }
    if (msg == NULL || len <= 0) {
        return -1;
    }
    if (oc_ctx->ops == NULL || oc_ctx->ops->report == NULL) {
        return -1;
    }
    return oc_ctx->ops->report(oc_ctx->handle, type, msg, len, timeout);
}

int oc_lwm2m_register(const char *name, const oc_ops_t *ops)
{
    if (name == NULL || ops == NULL || s_lwm2m.ops != NULL) {
        return -1;
    }
    s_lwm2m.name = name;
    s_lwm2m.ops  = ops;
    return 0;
}

int oc_lwm2m_unregister(const char *name)
{
    if (name == NULL || s_lwm2m.name == NULL) {
        return -1;
    }

    if (strcmp(name, s_lwm2m.name) == 0) {
        s_lwm2m.name = NULL;
        s_lwm2m.ops  = NULL;
        return 0;
    }
    return -1;
}