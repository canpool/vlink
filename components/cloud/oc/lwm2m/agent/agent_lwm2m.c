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

#define SERVER_ID   123

#define BINARY_APP_DATA_OBJECT_ID           19
#define BINARY_APP_DATA_RES_ID              0
#define BINARY_APP_DATA_OBJECT_INSTANCE_NUM 2

#define APP_UP_CHANNEL_URI      "/19/0/0"
#define APP_DOWN_CHANNEL_URI    "/19/1/0"

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

static int agent_dealer_callback(int op, const char *uri, char *msg, int len)
{
    int ret = -1;

    vlog_print("uri: %s, op: %d\n", uri, op);

    if (strcmp(uri, APP_DOWN_CHANNEL_URI) == 0) {
        switch (op)
        {
        case LWM2M_AL_OP_WRITE:
            ret = lwm2m_agent_receive(OC_LWM2M_MSG_APP_WRITE, msg, len);
            break;
        case LWM2M_AL_OP_EXCUTE:
            ret = lwm2m_agent_receive(OC_LWM2M_MSG_APP_EXECUTE, msg, len);
            break;
        case LWM2M_AL_OP_DISCOVER:
            ret = lwm2m_agent_receive(OC_LWM2M_MSG_APP_DISCOVER, msg, len);
            break;
        default:
            break;
        }
    }
    return ret;
}

static int agent_add_security_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_security_data_t data;
    lwm2m_al_uri_t uri;

#define URI_MAX_LEN 64
    char serverUri[URI_MAX_LEN] = {0};
    int i = 0;

    const uint8_t INS_IOT_SERVER_FLAG = 0x01;
    const uint8_t INS_BS_SERVER_FLAG = 0x02;
    uint8_t ins_flag = 0x00;
    uint8_t total_ins = 1;
    uint8_t index = 0;

    switch (config->bs_mode)
    {
    case OC_LWM2M_BS_FACTORY:
        ins_flag |= INS_IOT_SERVER_FLAG;
        total_ins = 1;
        break;
    case OC_LWM2M_BS_CLIENT_INITIATED:
        ins_flag |= INS_BS_SERVER_FLAG;
        total_ins = 1;
        break;
    default:
        return -1;
    }

    memset(&data, 0, sizeof(lwm2m_al_security_data_t));
    memset(&uri, 0, sizeof(lwm2m_al_uri_t));

    uri.obj_id = LWM2M_AL_SECURITY_OBJECT_ID;

    char *psk = (char *)config->server[index].psk_key;
    // at most, have two instance. in fact
    for (i = 0; i < total_ins; ++i)
    {
        if ((ins_flag & INS_IOT_SERVER_FLAG) && (ins_flag & INS_BS_SERVER_FLAG)) {
            uri.inst_id = i;   //i=0 for iot_server, i=1 for bs_server
            data.isBootstrap = ((i == 0) ? (false) : (true));
            index = i;
        } else {
            if (ins_flag & INS_IOT_SERVER_FLAG) {
                uri.inst_id = 0;
                data.isBootstrap = false;
                index = 0;
            }
            else  //if(ins_flag & INS_BS_SERVER_FLAG)  //even if not set INS_BS_SERVER_FLAG, still run in a certain process.
            {
                uri.inst_id = 1;
                data.isBootstrap = true;
                index = 1;
            }
        }

        if (config->server[index].host && config->server[index].port)  {
            if (psk != NULL) {
                snprintf(serverUri, URI_MAX_LEN, "coaps://%s:%s",
                    config->server[index].host, config->server[index].port);
            } else {
                snprintf(serverUri, URI_MAX_LEN, "coap://%s:%s",
                    config->server[index].host, config->server[index].port);
            }
        }
        data.serverUri = serverUri;

        data.publicIdentity = (char *)config->server[index].psk_identity;
        data.publicIdLen = (uint16_t)config->server[index].psk_identity_len;
        data.secretKey = psk;
        data.secretKeyLen = (uint16_t)config->server[index].psk_key_len;
        data.shortID = SERVER_ID;
        data.clientHoldOffTime = config->hold_off_time;

        if (lwm2m_al_add_object(m2m, &uri, (uintptr_t)&data) != 0) {
            return -1;
        }
    }
    return 0;
}

static int agent_add_server_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_server_data_t data;
    lwm2m_al_uri_t uri;

    data.binding = config->binding;
    data.lifetime = config->lifetime;
    data.shortServerId = SERVER_ID;
    data.storing = (config->storing_cnt != 0);

    // /1/0
    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_SERVER_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, (uintptr_t)&data);
}

static int agent_add_acc_ctrl_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_uri_t uri;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_ACL_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, 0);
}

static int agent_add_device_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_uri_t uri;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_DEVICE_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, 0);
}

static int agent_add_conn_m_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_uri_t uri;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_CONN_MONITOR_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, 0);
}

static int agent_add_firmware_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_uri_t uri;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_FIRMWARE_UPDATE_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, 0);
}

static int agent_add_location_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_uri_t uri;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = LWM2M_AL_LOCATION_OBJECT_ID;
    uri.inst_id = 0;

    return lwm2m_al_add_object(m2m, &uri, 0);
}

static int agent_add_binary_app_data_object(lwm2mer_t m2m, oc_config_t *config)
{
    lwm2m_al_app_data_t data;
    lwm2m_al_uri_t uri;
    int i;

    memset(&data, 0, sizeof(lwm2m_al_app_data_t));
    data.storingCnt = (uint16_t)config->storing_cnt;

    memset(&uri, 0, sizeof(lwm2m_al_uri_t));
    uri.obj_id = BINARY_APP_DATA_OBJECT_ID;

    // /19/0/0, /19/1/0
    for (i = 0; i < BINARY_APP_DATA_OBJECT_INSTANCE_NUM; ++i) {
        uri.inst_id = i;
        if (lwm2m_al_add_object(m2m, &uri, (uintptr_t)&data) != 0) {
            return -1;
        }
    }
    return 0;
}

static int agent_add_objects(lwm2mer_t m2m, oc_config_t *config)
{
    if (agent_add_security_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_server_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_acc_ctrl_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_device_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_conn_m_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_firmware_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_location_object(m2m, config) != 0) {
        return -1;
    }
    if (agent_add_binary_app_data_object(m2m, config) != 0) {
        return -1;
    }
    return 0;
}

int agent_op_init(uintptr_t *handle, oc_config_t *config)
{
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

    al_config.endpoint = config->endpoint;
    al_config.dealer = agent_dealer_callback;
    al_config.usr_data = config->user_data;

    config->binding = "UQ";
    config->lifetime = 20;
    config->storing_cnt = 5;
    config->hold_off_time = 10;

    if (lwm2m_al_init(&agent->m2m, &al_config) != 0) {
        goto EXIT_FREE_AGENT;
    }
    if (agent_add_objects(agent->m2m, config) != 0) {
        goto EXIT_DESTROY_LWM2M;
    }
    if (lwm2m_al_connect(agent->m2m) != 0) {
        goto EXIT_DESTROY_LWM2M;
    }

    s_agent_lwm2m = agent;
    *handle = (uintptr_t)agent;

    return 0;

EXIT_DESTROY_LWM2M:
    lwm2m_al_destroy(agent->m2m);
EXIT_FREE_AGENT:
    vos_free(agent);
    return -1;
}

int agent_op_destroy(uintptr_t handle)
{
    agent_lwm2m_t *agent = (agent_lwm2m_t *)handle;

    lwm2m_al_destroy(agent->m2m);

    vos_free(agent);
    s_agent_lwm2m = NULL;

    return 0;
}

int agent_op_report(uintptr_t handle, int type, char *msg, int len, uint32_t timeout)
{
    agent_lwm2m_t *agent = (agent_lwm2m_t *)handle;
    const char *uri = NULL;

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