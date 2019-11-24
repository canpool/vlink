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
    uintptr_t       handle;
    const oc_ops_t *ops;
} oc_lwm2m_context_t;

static oc_lwm2m_t s_lwm2m;

int oc_lwm2m_init(oc_context_t *ctx, oc_config_t *config)
{
    oc_lwm2m_context_t *oc_lwm2m = NULL;

    if (ctx == NULL || config == NULL) {
        return -1;
    }

    if (s_lwm2m.ops == NULL || s_lwm2m.ops->init == NULL) {
        return -1;
    }

    oc_lwm2m = (oc_lwm2m_context_t *)vos_zalloc(sizeof(oc_lwm2m_context_t));
    if (oc_lwm2m == NULL) {
        return -1;
    }
    if (s_lwm2m.ops->init(&oc_lwm2m->handle, config) != 0) {
        vos_free(oc_lwm2m);
        return -1;
    }
    oc_lwm2m->ops = s_lwm2m.ops;
    *ctx = (oc_context_t)oc_lwm2m;

    return 0;
}

int oc_lwm2m_destroy(oc_context_t ctx)
{
    oc_lwm2m_context_t *oc_lwm2m = (oc_lwm2m_context_t *)ctx;

    if (oc_lwm2m == NULL || oc_lwm2m->ops == NULL || oc_lwm2m->ops->destroy == NULL) {
        return -1;
    }
    if (oc_lwm2m->ops->destroy(oc_lwm2m->handle) == 0) {
        vos_free(oc_lwm2m);
        return 0;
    }
    return -1;
}

int oc_lwm2m_report(oc_context_t ctx, int type, char *msg, int len, uint32_t timeout)
{
    oc_lwm2m_context_t *oc_lwm2m = (oc_lwm2m_context_t *)ctx;

    if (oc_lwm2m == NULL || oc_lwm2m->ops == NULL || oc_lwm2m->ops->report == NULL) {
        return -1;
    }
    return oc_lwm2m->ops->report(oc_lwm2m->handle, type, msg, len, timeout);
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