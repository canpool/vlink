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

#include "agent_coap.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "oc_coap.h"
#include "coap_al.h"
#include "vos.h"

typedef struct {
    char            *server_ip;
    char            *server_port;

    unsigned char   *psk;
    size_t           psk_len;
    unsigned char   *psk_id;
    size_t           psk_id_len;
} agent_server_t;

typedef struct {
    oc_config_t     config;
    agent_server_t  server[2];
    vtask_t         task;
    coaper_t        coaper;
} agent_coap_t;

#define AGENT_MAX_SEND_RETRY    500
#define AGENT_MAX_TOKEN_LEN     8

#define AGENT_MAX_EPID_LEN      50

// OC support:
// </bs>,</rd>,</t>,</t/r>,</h>,</h/e>,</firmwarePackage>,</.well-known/core>

/* agent local resources */
unsigned char agent_res[3]  = {'t'};
unsigned char agent_res1[3] = {'r'};
unsigned char agent_res2[AGENT_MAX_EPID_LEN] = "ep=";

int g_bind_finsh = 0;
static int  s_agent_state = AGENT_STAT_INIT;
static int  s_agent_errno = 0;
static char s_agent_stop_flag = 0;

static int agent_errno_set(int err)
{
    s_agent_errno = err;
    return AGENT_OK;
}

int agent_errno_get(void)
{
    return s_agent_errno;
}

static int agent_state_flow(int *state)
{
    switch (*state) {
    case AGENT_STAT_INIT:
        *state = AGENT_STAT_BINDING;
        break;
    case AGENT_STAT_BINDING:
        *state = AGENT_STAT_WAIT_BINDOK;
        break;
    case AGENT_STAT_WAIT_BINDOK:
        *state = AGENT_STAT_IDLE;
        break;
    case AGENT_STAT_IDLE:
        *state = AGENT_STAT_IDLE;
        break;
    case AGENT_STAT_ERR:
        *state = AGENT_STAT_BINDING;
        break;
    default:
        *state = AGENT_STAT_ERR;
        break;
    }
    return AGENT_OK;
}

static int agent_state_get(void *res)
{
    return s_agent_state;
}

static int agent_state_set(int state)
{
    if ((state >= AGENT_STAT_INIT) && (state < AGENT_STAT_MAX)) {
        s_agent_state = state;
    }
    return AGENT_OK;
}

static int agent_coap_step_init(agent_coap_t *agent)
{
    coap_al_config_t config;

    memset(&config, 0, sizeof(coap_al_config_t));

    config.host = agent->config.app_server.host;
    config.port = agent->config.app_server.port;
    config.dealer = agent->config.recv;
    config.psk_key = agent->config.app_server.psk;
    config.psk_key_len = agent->config.app_server.psk_len;
    config.psk_identity = agent->config.app_server.psk_id;
    config.psk_identity_len = agent->config.app_server.psk_id_len;

    return coap_al_init(&agent->coaper, &config);
}

static int agent_coap_step_binding(agent_coap_t *agent)
{
    size_t ed_len = strlen(agent->config.app_server.endpoint);
    void *msg = NULL;

    coap_al_add_option(agent->coaper, COAP_AL_OPTION_URI_PATH, 1, (uint8_t *)agent_res);
    coap_al_add_option(agent->coaper, COAP_AL_OPTION_URI_PATH, 1, (uint8_t *)agent_res1);

    memcpy(agent_res2 + 3, agent->config.app_server.endpoint, ed_len);

    coap_al_add_option(agent->coaper, COAP_AL_OPTION_URI_QUERY, ed_len + 3, (uint8_t *)agent_res2);

    if ((msg = coap_al_request(agent->coaper, COAP_AL_MESSAGE_CON, COAP_AL_REQUEST_POST, NULL, 0)) == NULL) {
        return -1;
    }

    if (coap_al_send(agent->coaper, msg) < 0) {
        return -1;
    }
    return 0;
}

static int agent_coap(uintptr_t args)
{
    agent_coap_t *agent = (agent_coap_t *)args;
    int ret = 0;
    int startrcvmsg = 0;

    unsigned long long bindstarttm;
    unsigned long long curtm;

    while (1) {
        /* stop agent */
        if (s_agent_stop_flag) {
            coap_al_destroy(agent->coaper);
            s_agent_stop_flag = 0;
            break;
        }

        switch (agent_state_get(NULL)) {
            case AGENT_STAT_INIT: {
                agent_coap_step_init(agent);
                agent_state_flow(&s_agent_state);
                break;
            }
            case AGENT_STAT_BINDING: {
                if (agent_coap_step_binding(agent) != 0) {
                    agent_state_set(AGENT_STAT_BINDING);
                    break;
                }
                agent_state_flow(&s_agent_state);
                startrcvmsg = 1;
                bindstarttm = vos_sys_time();
                break;
            }
            case AGENT_STAT_WAIT_BINDOK: {
                curtm = vos_sys_time();
                if (curtm - bindstarttm >= AGENT_BIND_TIMEOUT) {
                    g_bind_finsh = 0;
                    agent_state_set(AGENT_STAT_BINDING);
                    startrcvmsg = 0;
                    agent_errno_set(AGENT_ERR_CODE_BINDTIMEOUT);
                    break;
                }
                if (g_bind_finsh) {
                    agent_state_flow(&s_agent_state);
                }
                break;
            }
            case AGENT_STAT_IDLE: {
                ret = coap_al_send(agent->coaper, NULL);
                break;
            }
            case AGENT_STAT_ERR:
            default: {
                agent_state_set(AGENT_STAT_BINDING);
                startrcvmsg = 0;
                g_bind_finsh = 0;
                break;
            }
        }
        /* agent task should recive all coap messages. */
        if (startrcvmsg) {
            ret = coap_al_recv(agent->coaper);
            if (ret < 0) {
                agent_errno_set(AGENT_ERR_CODE_NETWORK);
                vos_task_sleep(100);
            }
        }
    }
    return AGENT_OK;
}

static int agent_op_init(uintptr_t *handle, oc_config_t *config)
{
    agent_coap_t *agent = NULL;

    agent = (agent_coap_t *)vos_zalloc(sizeof(agent_coap_t));
    if (agent == NULL) {
        return -1;
    }

    agent->config = *config;

    agent_server_t *iot_server = &agent->server[0];
    agent_server_t *bs_server  = &agent->server[1];

    iot_server->server_ip   = config->app_server.host;
    iot_server->server_port = config->app_server.port;
    iot_server->psk         = config->app_server.psk;
    iot_server->psk_len     = config->app_server.psk_len;
    iot_server->psk_id      = config->app_server.psk_id;
    iot_server->psk_id_len  = config->app_server.psk_id_len;

    bs_server->server_ip   = config->bs_server.host;
    bs_server->server_port = config->bs_server.port;
    bs_server->psk         = config->bs_server.psk;
    bs_server->psk_len     = config->bs_server.psk_len;
    bs_server->psk_id      = config->bs_server.psk_id;
    bs_server->psk_id_len  = config->bs_server.psk_id_len;

    if (vos_task_create(&agent->task, "agent_coap", agent_coap, (uintptr_t)agent, 0x1000, 10) != 0) {
        vos_free(agent);
        return -1;
    }
    *handle = (uintptr_t)agent;

    return 0;
}

static int agent_op_destroy(uintptr_t handle)
{
    agent_coap_t *agent = (agent_coap_t *)handle;

    /* set agent to stop send data */
    s_agent_stop_flag = 1;
    /* wait agent data process task finish recieve data and exit */
    vos_task_sleep(1000);

    vos_task_delete(&agent->task);

    vos_free(agent);

    return 0;
}

static int agent_op_report(uintptr_t handle, char *buf, int len)
{
    int ret = 0;
    agent_coap_t *agent = (agent_coap_t *)handle;
    void *msg = NULL;
    static unsigned char lifetime = 0;

    while (agent_state_get(NULL) != AGENT_STAT_IDLE) {
        ret++;
        vos_task_sleep(10);
        if (ret >= AGENT_MAX_SEND_RETRY) {
            return AGENT_ERR;
        }
    }

    lifetime++;
    coap_al_add_option(agent->coaper, COAP_AL_OPTION_OBSERVE, 1, (uint8_t *)&lifetime);

    if ((msg = coap_al_request(agent->coaper, COAP_AL_MESSAGE_NON,
            COAP_AL_RESP_CODE(205), (uint8_t *)buf, len)) == NULL) {
        return AGENT_ERR;
    }
    if (coap_al_send(agent->coaper, msg) != 0) {
        return AGENT_ERR;
    }

    return AGENT_OK;
}

const oc_ops_t  s_oc_coap_agent = {
    .init       = agent_op_init,
    .destroy    = agent_op_destroy,
    .report     = agent_op_report,
};

int oc_coap_setup(void)
{
    return oc_coap_register("oc_coap_agent" ,&s_oc_coap_agent);
}
