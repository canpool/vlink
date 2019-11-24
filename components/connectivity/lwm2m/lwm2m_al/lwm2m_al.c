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

typedef struct {
    const lwm2m_al_ops_t    *ops;
} lwm2m_al_cb_t;

static lwm2m_al_cb_t s_lwm2m_al;

int lwm2m_al_init(lwm2mer_t *m2m, lwm2m_al_config_t *config)
{
    if (m2m == NULL || config == NULL) {
        return -1;
    }
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->init == NULL) {
        return -1;
    }
    return s_lwm2m_al.ops->init(m2m, config);
}

int lwm2m_al_destroy(lwm2mer_t m2m)
{
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->destroy == NULL) {
        return -1;
    }
    if (s_lwm2m_al.ops->destroy(m2m) != 0) {
        return -1;
    }
    return 0;
}

int lwm2m_al_send(lwm2mer_t m2m, const char *uri, const char *msg, int len, uint32_t timeout)
{
    if (s_lwm2m_al.ops == NULL || s_lwm2m_al.ops->send == NULL) {
        return -1;
    }
    if (s_lwm2m_al.ops->send(m2m, uri, msg, len, timeout) != 0) {
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