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

#include "oc_coap.h"
#include "vos.h"

#include <string.h>

typedef struct {
    const char      *name;
    const oc_ops_t  *ops;
} oc_coap_t;

typedef struct {
    uintptr_t       handle;
    const oc_ops_t *ops;
} oc_coap_context_t;

static oc_coap_t s_coap;

int oc_coap_init(oc_context_t *ctx, oc_config_t *config)
{
    oc_coap_context_t *oc_coap = NULL;

    if (ctx == NULL || config == NULL) {
        return -1;
    }

    if (s_coap.ops == NULL || s_coap.ops->init == NULL) {
        return -1;
    }

    oc_coap = (oc_coap_context_t *)vos_zalloc(sizeof(oc_coap_context_t));
    if (oc_coap == NULL) {
        return -1;
    }
    if (s_coap.ops->init(&oc_coap->handle, config) != 0) {
        vos_free(oc_coap);
        return -1;
    }
    oc_coap->ops = s_coap.ops;
    *ctx = (oc_context_t)oc_coap;

    return 0;
}

int oc_coap_destroy(oc_context_t ctx)
{
    oc_coap_context_t *oc_coap = (oc_coap_context_t *)ctx;

    if (oc_coap == NULL || oc_coap->ops == NULL || oc_coap->ops->destroy == NULL) {
        return -1;
    }
    if (oc_coap->ops->destroy(oc_coap->handle) == 0) {
        vos_free(oc_coap);
        return 0;
    }
    return -1;
}

int oc_coap_report(oc_context_t ctx, char *msg, int len)
{
    oc_coap_context_t *oc_coap = (oc_coap_context_t *)ctx;

    if (oc_coap == NULL || oc_coap->ops == NULL || oc_coap->ops->report == NULL) {
        return -1;
    }
    return oc_coap->ops->report(oc_coap->handle, msg, len);
}

int oc_coap_register(const char *name, const oc_ops_t *ops)
{
    if (name == NULL || ops == NULL || s_coap.ops != NULL) {
        return -1;
    }
    s_coap.name = name;
    s_coap.ops  = ops;
    return 0;
}

int oc_coap_unregister(const char *name)
{
    if (name == NULL || s_coap.name == NULL) {
        return -1;
    }

    if (strcmp(name, s_coap.name) == 0) {
        s_coap.name = NULL;
        s_coap.ops  = NULL;
        return 0;
    }
    return -1;
}