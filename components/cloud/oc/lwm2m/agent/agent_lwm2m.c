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

#include "agent_lwm2m.h"

#include <string.h>

#include "oc_lwm2m.h"
#include "lwm2m_al.h"
#include "vos.h"

typedef struct {
    oc_config_t config;
    lwm2mer_t   m2m;
} agent_lwm2m_t;

static agent_lwm2m_t *s_agent_lwm2m = NULL;

int lwm2m_agent_receive(oc_lwm2m_msg_e type, char *msg, int len)
{
    if (s_agent_lwm2m == NULL || s_agent_lwm2m->config.recv_func == NULL) {
        return -1;
    }
    s_agent_lwm2m->config.recv_func(s_agent_lwm2m->config.user_data, type, msg, len);
    return 0;
}


int agent_op_init(uintptr_t *handle, oc_config_t *config)
{
    if (handle == NULL || config == NULL) {
        return -1;
    }
    if (s_agent_lwm2m != NULL) {
        return -1;
    }

    agent_lwm2m_t *agent = (agent_lwm2m_t *)vos_zalloc(sizeof(agent_lwm2m_t));
    if (agent == NULL) {
        return -1;
    }
    agent->config = *config;

    lwm2m_al_config_t al_config;
    memset(&al_config, 0, sizeof(lwm2m_al_config_t));

    // al_config.endpoint = config->endpoint;
    // al_config.manufacturer = config->manufacturer;
    // al_config.dev_type = config->dev_type;
    al_config.endpoint = config->endpoint;
    al_config.manufacturer = "Agent_Tiny";
    al_config.dev_type = "Lwm2mFota";

    // al_config.binding = config->binding;
    // al_config.lifetime = config->lifetime;
    // al_config.storing_cnt = config->storing_cnt;
    al_config.binding = "UQ";
    al_config.lifetime = 20;
    al_config.storing_cnt = 5;
    al_config.bs_mode = config->bs_mode;
    // al_config.hold_off_time = config->hold_off_time;
    al_config.hold_off_time = 10;

    memcpy(al_config.server, config->server, sizeof(config->server));
    al_config.usr_data = config->user_data;

    if (lwm2m_al_init(&agent->m2m, &al_config) != 0) {
        vos_free(agent);
        return -1;
    }

    s_agent_lwm2m = agent;
    *handle = (uintptr_t)agent;

    return 0;
}

int agent_op_destroy(uintptr_t handle)
{
    agent_lwm2m_t *agent = (agent_lwm2m_t *)handle;

    if (agent == NULL || agent != s_agent_lwm2m) {
        return -1;
    }

    lwm2m_al_destroy(agent->m2m);

    vos_free(agent);
    s_agent_lwm2m = NULL;

    return 0;
}

int agent_op_report(uintptr_t handle, int type, char *msg, int len, uint32_t timeout)
{
    agent_lwm2m_t *agent = (agent_lwm2m_t *)handle;
    const char *uri = NULL;

    if (agent == NULL || agent != s_agent_lwm2m || msg == NULL || len <= 0) {
        return -1;
    }

    switch (type)
    {
    case FIRMWARE_UPDATE_STATE:
        uri = "/5/0/3";
        break;
    case APP_DATA:
        uri = "/19/0/0";
        break;
    default:
        return -1;
    }

    return lwm2m_al_send(agent->m2m, uri, msg, len, timeout);
}

static const oc_ops_t s_oc_lwm2m_agent = {
    .init       = agent_op_init,
    .destroy    = agent_op_destroy,
    .report     = agent_op_report,
};

int oc_lwm2m_setup(void)
{
    return oc_lwm2m_register("oc_lwm2m_agent", &s_oc_lwm2m_agent);
}