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

#include "mqtt_al.h"
#include "vos.h"

typedef struct {
    const mqtt_al_ops_t *ops;
} mqtt_al_cb_t;

typedef struct {
    uintptr_t            magic;
    uintptr_t            handle;
} mqtt_al_context_t;

static mqtt_al_cb_t s_mqtt_al;

int mqtt_al_init(mqtter_t *mqtter)
{
    mqtt_al_context_t *al_ctx = NULL;

    if (mqtter == NULL) {
        return -1;
    }
    al_ctx = (mqtt_al_context_t *)(*mqtter);
    if (magic_verify(al_ctx)) {
        return -1;
    }
    if (s_mqtt_al.ops == NULL || s_mqtt_al.ops->init == NULL) {
        return -1;
    }

    al_ctx = (mqtt_al_context_t *)vos_zalloc(sizeof(mqtt_al_context_t));
    if (al_ctx == NULL) {
        return -1;
    }

    if (s_mqtt_al.ops->init(&al_ctx->handle) != 0) {
        vos_free(al_ctx);
        return -1;
    }
    al_ctx->magic = (uintptr_t)al_ctx;
    *mqtter = (mqtter_t)al_ctx;

    return 0;
}

int mqtt_al_destroy(mqtter_t mqtter)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_mqtt_al.ops == NULL || s_mqtt_al.ops->destroy == NULL) {
        return -1;
    }
    if (s_mqtt_al.ops->destroy(al_ctx->handle) != 0) {
        return -1;
    }
    al_ctx->magic = 0;
    vos_free(al_ctx);

    return 0;
}

int mqtt_al_connect(mqtter_t mqtter, mqtt_al_conn_t *con)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (con == NULL || s_mqtt_al.ops == NULL || s_mqtt_al.ops->connect == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->connect(al_ctx->handle, con);
}

int mqtt_al_disconnect(mqtter_t mqtter)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_mqtt_al.ops == NULL || s_mqtt_al.ops->disconnect == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->disconnect(al_ctx->handle);
}

int mqtt_al_publish(mqtter_t mqtter, mqtt_al_pub_t *pub)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (pub == NULL || s_mqtt_al.ops == NULL || s_mqtt_al.ops->publish == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->publish(al_ctx->handle, pub);
}

int mqtt_al_subscribe(mqtter_t mqtter, mqtt_al_sub_t *sub)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (sub == NULL || s_mqtt_al.ops == NULL || s_mqtt_al.ops->subscribe == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->subscribe(al_ctx->handle, sub);
}

int mqtt_al_unsubscribe(mqtter_t mqtter, mqtt_al_unsub_t *unsub)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (unsub == NULL || s_mqtt_al.ops == NULL || s_mqtt_al.ops->unsubscribe == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->unsubscribe(al_ctx->handle, unsub);
}

int mqtt_al_checkstatus(mqtter_t mqtter)
{
    mqtt_al_context_t *al_ctx = (mqtt_al_context_t *)(mqtter);

    if (!magic_verify(al_ctx)) {
        return -1;
    }
    if (s_mqtt_al.ops == NULL || s_mqtt_al.ops->checkstatus == NULL) {
        return -1;
    }
    return s_mqtt_al.ops->checkstatus(al_ctx->handle);
}

int mqtt_al_install(const mqtt_al_ops_t *ops)
{
    if (ops == NULL || s_mqtt_al.ops != NULL) {
        return -1;
    }
    s_mqtt_al.ops = ops;
    return 0;
}

int mqtt_al_uninstall(void)
{
    if (s_mqtt_al.ops == NULL) {
        return -1;
    }
    s_mqtt_al.ops = NULL;
    return 0;
}

int mqtt_al_checkfilter(const mqtt_al_string_t *topic_filter)
{
    // All Topic Names and Topic Filters MUST be at least one character long (v3.1.1 #4.7.3)
    if (topic_filter == NULL || topic_filter->data == NULL || topic_filter->len <= 0) {
        return -1;
    }
    char *f = topic_filter->data;
    int len = topic_filter->len;
    int pos = 0;

    while (pos < len) {
        if (f[pos] == '+') {
            /**
             * The single-level wildcard can be used at any level in the Topic Filter, including first and
             * last levels. Where it is used it MUST occupy an entire level of the filter
             * valid format: "+", "+/+", "+/aa", "aa/+", "aa/+/bb"
             */
            // bad format: '+' is not first, but prev is not '/', eg. "foo+" or "foo+/a"
            if (pos > 0 && f[pos - 1] != '/') {
                return -1;
            }
            // bad format: '+' is not last one, but next is not '/', eg. "+foo" or "a/+foo"
            if (pos < len - 1 && f[pos + 1] != '/') {
                return -1;
            }
            if (pos + 1 == len) {
                return 0;
            }
        } else if (f[pos] == '#') {
            /**
             * The multi-level wildcard character MUST be specified either on its own or following a topic
             * level separator. In either case it MUST be the last character specified in the Topic Filter
             * valid format: "#", "aa/#"
             */
            // bad format: '#' is not first, but prev is not '/', eg. "foo#"
            if (pos > 0 && f[pos - 1] != '/') {
                return -1;
            }
            // bad format: '#' is not last one, eg. "#/food"
            if (pos + 1 != len) {
                return -1;
            } else {
                return 0;
            }
        } else if (f[pos] == '$') {
            /**
             * (v3.1.1 #4.7.2) Topics beginning with $
             * The Server MUST NOT match Topic Filters starting with a wildcard character (# or +) with Topic Names
             * beginning with a $ character
             * Applications cannot use a topic with a leading $ character for their own purposes
             */
            if (pos > 0) {
                return -1;
            }
        }
        pos++;
    }

    return 0;
}

int mqtt_al_checkname(const mqtt_al_string_t *topic_name)
{
    if (topic_name == NULL || topic_name->data == NULL || topic_name->len <= 0) {
        return -1;
    }
    char *name = topic_name->data;
    int len = topic_name->len;
    int pos = 0;

    while (pos < len) {
        if (name[pos] == '+' || name[pos] == '#') {
            return -1;
        } else if (name[pos] == '$') {
            if (pos > 0) {
                return -1;
            }
        }
        pos++;
    }

    return 0;
}
