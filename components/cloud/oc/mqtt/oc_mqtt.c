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

#include "oc_mqtt.h"
#include "vos.h"

#include <string.h>

typedef struct {
    const char      *name;
    const oc_ops_t  *ops;
} oc_mqtt_t;

typedef struct {
    uintptr_t        handle;
    const oc_ops_t  *ops;
} oc_mqtt_context_t;

static oc_mqtt_t s_mqtt;

int oc_mqtt_init(oc_context_t *ctx, oc_config_t *config)
{
    oc_mqtt_context_t *oc_mqtt = NULL;

    if (ctx == NULL || config == NULL) {
        return -1;
    }

    if (s_mqtt.ops == NULL || s_mqtt.ops->init == NULL) {
        return -1;
    }

    oc_mqtt = (oc_mqtt_context_t *)vos_zalloc(sizeof(oc_mqtt_context_t));
    if (oc_mqtt == NULL) {
        return -1;
    }
    if (s_mqtt.ops->init(&oc_mqtt->handle, config) != 0) {
        vos_free(oc_mqtt);
        return -1;
    }
    oc_mqtt->ops = s_mqtt.ops;
    *ctx = (oc_context_t)oc_mqtt;

    return 0;
}

int oc_mqtt_destroy(oc_context_t ctx)
{
    oc_mqtt_context_t *oc_mqtt = (oc_mqtt_context_t *)ctx;

    if (oc_mqtt == NULL || oc_mqtt->ops == NULL || oc_mqtt->ops->destroy == NULL) {
        return -1;
    }
    if (oc_mqtt->ops->destroy(oc_mqtt->handle) == 0) {
        vos_free(oc_mqtt);
        return 0;
    }
    return -1;
}

int oc_mqtt_report(oc_context_t ctx, char *msg, int len, int qos)
{
    oc_mqtt_context_t *oc_mqtt = (oc_mqtt_context_t *)ctx;

    if (msg == NULL || oc_mqtt == NULL || oc_mqtt->ops == NULL || oc_mqtt->ops->report == NULL) {
        return -1;
    }
    return oc_mqtt->ops->report(oc_mqtt->handle, msg, len, qos);
}

int oc_mqtt_register(const char *name, const oc_ops_t *ops)
{
    if (name == NULL || ops == NULL || s_mqtt.ops != NULL) {
        return -1;
    }
    s_mqtt.name = name;
    s_mqtt.ops  = ops;
    return 0;
}

int oc_mqtt_unregister(const char *name)
{
    if (name == NULL || s_mqtt.name == NULL) {
        return -1;
    }

    if (strcmp(name, s_mqtt.name) == 0) {
        s_mqtt.name = NULL;
        s_mqtt.ops  = NULL;
        return 0;
    }
    return -1;
}