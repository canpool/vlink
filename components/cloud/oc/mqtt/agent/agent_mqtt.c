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

#include "agent_mqtt.h"

#include <string.h>

#include "oc_mqtt.h"
#include "mqtt_al.h"
#include "vos.h"

#include "vsl_hmac.h"
#include "cJSON.h"


// https://support.huaweicloud.com/api-iothub/iot_06_3002.html
// iot -> dev, /huawei/v1/devices/{deviceId}/command/{codecMode}
#define MQTT_DMP_DATA_COMMAND_FMT       "/huawei/v1/devices/%s/command/%s"
// dev -> iot, /huawei/v1/devices/{deviceId}/data/{codecMode}
#define MQTT_DMP_DATA_REPORT_FMT        "/huawei/v1/devices/%s/data/%s"

#define MQTT_SECRET_NOTIFY_TOPIC_FMT    "/huawei/v1/products/%s/sn/%s/secretNotify"
#define MQTT_SECRET_ACK_TOPIC_FMT       "/huawei/v1/products/%s/sn/%s/secretACK"

// https://support.huaweicloud.com/api-iotps/iot_03_0001.html
// dev -> iot, /huawei/v1/devices/{nodeId}/iodpsData
#define MQTT_BS_DATA_REPORT_FMT         "/huawei/v1/devices/%s/iodpsData"
// iot -> dev, /huawei/v1/devices/{nodeId}/iodpsCommand
#define MQTT_BS_DATA_COMMAND_FMT        "/huawei/v1/devices/%s/iodpsCommand"

/* The unit is millisecond */
#define MQTT_CON_BACKOFF_TIME           (1000)
#define MQTT_CON_BACKOFF_MAXTIMES       6

#define MQTT_CHECK_TIME_VALUE           "2018111517"

// deviceId_authType_signType_timestamp
#define MQTT_OC_CLIENTID_STATIC_FMT     "%s_%d_%d_%s"
#define MQTT_OC_CLIENTID_DYNAMIC_FMT    "%s_%s_%d_%d_%s"

#define MQTT_STRING_MAX_LEN             127

static const char *s_codec_mode[OC_MQTT_CODEC_MODE_MAX] = {"binary", "json"};

typedef struct {
    char    *dmp_addr; // get from the bs server
    char    *dmp_port; // get from the bs server
} agent_mqtt_bs_t;

typedef struct {
    oc_config_t     config;
    agent_mqtt_bs_t bs;
    vtask_t         task;

    mqtter_t        mqtter;
    char           *username;
    char           *password;
    char           *pub_topic;
    char           *sub_topic;
    char           *cli_id;
    char           *srv_addr;
    char           *srv_port;
    mqtt_al_dealer  dealer;

    /* state flag */
    unsigned int    flag_run            : 1;
    unsigned int    flag_stop           : 1;
    unsigned int    flag_bs_run         : 1;
    unsigned int    flag_bs_gotaddr     : 1;
    unsigned int    flag_dmp_run        : 1;
    unsigned int    flag_dmp_connected  : 1;
} agent_mqtt_t;

static agent_mqtt_t *s_agent_mqtt = NULL;

static int agent_handle_app_msg(uintptr_t handle, mqtt_al_rcv_t *msg)
{
    oc_mqtt_rcv_t oc_msg;

    memset(&oc_msg, 0, sizeof(oc_mqtt_rcv_t));
    oc_msg.topic = msg->topic.data;
    oc_msg.topic_len = msg->topic.len;
    oc_msg.data = msg->msg.data;
    oc_msg.data_len = msg->msg.len;
    oc_msg.qos = msg->qos;

    s_agent_mqtt->config.dealer((uintptr_t)s_agent_mqtt, &oc_msg);

    return 0;
}

static int agent_free_param(agent_mqtt_t *agent)
{
    if (mqtt_al_checkstatus(agent->mqtter) == MQTT_AL_ERR_NONE) {
        mqtt_al_disconnect(agent->mqtter);
    }

    safe_free(agent->username);
    safe_free(agent->password);
    safe_free(agent->sub_topic);
    safe_free(agent->pub_topic);
    safe_free(agent->cli_id);

    agent->srv_addr = NULL;
    agent->srv_port = NULL;

    return 0;
}

static int agent_handle_dmp_msg(uintptr_t handle, mqtt_al_rcv_t *msg)
{
    int     ret = -1;
    cJSON  *root = NULL;
    cJSON  *cmd_item = NULL;
    cJSON  *serviceid_item = NULL;
    char   *json_buf = NULL;
    int     reboot_servceid_match = 0;
    int     reboot_cmd_match = 0;
    int     mqtt_rebootstrap = 0;

    // for we must add the '/0' to the end to make sure the json parse correct
    if (msg == NULL || msg->msg.data == NULL || s_agent_mqtt->config.dealer == NULL) {
        return ret;
    }

    json_buf = vos_malloc(msg->msg.len + 1);
    if (json_buf == NULL) {
        return ret;
    }
    memcpy(json_buf, msg->msg.data, msg->msg.len);
    json_buf[msg->msg.len] = '\0';

    root = cJSON_Parse(json_buf);
    if (root == NULL) {
        goto EXIT_FREE_JSON_BUF;
    }

    cmd_item = cJSON_GetObjectItem(root, "cmd");
    if (cmd_item != NULL &&
        strncmp(cmd_item->valuestring, "BootstrapRequestTrigger", strlen(cmd_item->valuestring)) == 0) {
        reboot_cmd_match = 1;
    }
    serviceid_item = cJSON_GetObjectItem(root, "serviceid");
    if (serviceid_item != NULL &&
        strncmp(serviceid_item->valuestring, "IOTHUB.BS", strlen(serviceid_item->valuestring)) == 0) {
        reboot_servceid_match = 1;
    }
    if (reboot_cmd_match && reboot_servceid_match) {
        mqtt_rebootstrap = 1;
    }

    ret = 0;

    cJSON_Delete(root);

EXIT_FREE_JSON_BUF:
    vos_free(json_buf);

    if (mqtt_rebootstrap) { ///< we reboot it now
        vlog_print("THIS IS THE OC MQTT REBOOTSTRAP COMMAND");
        s_agent_mqtt->flag_dmp_run = 0;
        return ret;
    }

    agent_handle_app_msg(handle, msg);

    return ret;
}

static int agent_gen_dmp_param(agent_mqtt_t *agent)
{
    int len;
    int i;
    unsigned char mach[CONFIG_MACH_LEN] = {0};
    char *tv = MQTT_CHECK_TIME_VALUE;
    char *pwd;

    agent_free_param(agent);

    if (agent->config.dev_type == OC_MQTT_DEV_TYPE_STATIC) {
        // client id
        len = strlen(agent->config.dev_info.s.devid) + strlen(tv) +
            strlen(MQTT_OC_CLIENTID_STATIC_FMT) + 1;
        if ((agent->cli_id = (char *)vos_malloc(len)) == NULL) {
            goto EXIT_MALLOC;
        }
        snprintf(agent->cli_id, len, MQTT_OC_CLIENTID_STATIC_FMT, agent->config.dev_info.s.devid,
                agent->config.auth_type, agent->config.sign_type, tv);

        // username
        len = strlen(agent->config.dev_info.s.devid) + 1;
        if ((agent->username = (char *)vos_malloc(len)) == NULL) {
            goto EXIT_MALLOC;
        }
        memcpy(agent->username, agent->config.dev_info.s.devid, len);

        // password (hmac)
        len = sizeof(mach) * 2 + 1;
        pwd = (char *)agent->config.dev_info.s.devpwd;
        if ((agent->password = (char *)vos_malloc(len)) == NULL) {
            goto EXIT_MALLOC;
        }
        if (vsl_hmac((unsigned char *)tv, strlen(tv), (unsigned char *)pwd, strlen(pwd), mach) != 0) {
            goto EXIT_MALLOC;
        }
        for (i = 0; i < sizeof(mach); ++i) {
            // hexify, eg: 0x12 0x34 => "1234"
            snprintf(agent->password + (i << 1), 3, "%02x", mach[i]);
        }

        // subscribe topic
        len = strnlen(agent->config.dev_info.s.devid, MQTT_STRING_MAX_LEN) +
                strnlen(s_codec_mode[agent->config.codec_mode], MQTT_STRING_MAX_LEN) +
                strlen(MQTT_DMP_DATA_COMMAND_FMT) + 1;
        if ((agent->sub_topic = (char *)vos_malloc(len)) == NULL) {
            goto EXIT_MALLOC;
        }
        snprintf(agent->sub_topic, len, MQTT_DMP_DATA_COMMAND_FMT,
                agent->config.dev_info.s.devid, s_codec_mode[agent->config.codec_mode]);

        // publish topic
        len = strnlen(agent->config.dev_info.s.devid, MQTT_STRING_MAX_LEN) +
                strnlen(s_codec_mode[agent->config.codec_mode], MQTT_STRING_MAX_LEN) +
                strlen(MQTT_DMP_DATA_REPORT_FMT) + 1;
        if ((agent->pub_topic = (char *)vos_malloc(len)) == NULL) {
            goto EXIT_MALLOC;
        }
        snprintf(agent->pub_topic, len, MQTT_DMP_DATA_REPORT_FMT,
                agent->config.dev_info.s.devid, s_codec_mode[agent->config.codec_mode]);

        if (agent->config.bs_mode == OC_MQTT_BS_MODE_CLIENT_INITIALIZE) {
            agent->srv_addr = agent->bs.dmp_addr;
            agent->srv_port = agent->bs.dmp_port;
        } else {
            agent->srv_addr = (char *)agent->config.host;
            agent->srv_port = (char *)agent->config.port;
        }
        agent->dealer = agent_handle_dmp_msg;
    } else {
        return -1;
    }
    return 0;

EXIT_MALLOC:
    agent_free_param(agent);
    return -1;
}

// https://support.huaweicloud.com/api-iotps/iot_03_0006.html
//
// {
//     "address": "10.0.0.1:8883",
//     "dnsFlag": 1
// }
static int agent_handle_bs_msg(uintptr_t handle, mqtt_al_rcv_t *msg)
{
    int     ret = -1;
    cJSON  *root = NULL;
    cJSON  *addr_item = NULL;
    char   *port = NULL;
    int     port_len;
    int     server_len;
    char   *server = NULL;
    char   *json_buf = NULL;

    vlog_print("bs topic:%s qos:%d", msg->topic.data, msg->qos);

    json_buf = (char *)vos_malloc(msg->msg.len + 1);
    if (json_buf == NULL) {
        return ret;
    }
    memcpy(json_buf, msg->msg.data, msg->msg.len);
    json_buf[msg->msg.len] = '\0';

    vlog_print("msg:%s", json_buf);
    root = cJSON_Parse(json_buf);
    if (root == NULL) {
        goto EXIT_FREE_JSON_BUF;
    }
    addr_item = cJSON_GetObjectItem(root, "address");
    if (addr_item == NULL) {
        goto EXIT_DEL_JSON_OBJ;
    }
    vlog_print("address:%s", addr_item->valuestring);
    port = strrchr(addr_item->valuestring, ':');
    if (port == NULL) {
        goto EXIT_DEL_JSON_OBJ;
    }
    server = addr_item->valuestring;
    server_len = port - addr_item->valuestring;
    port++;
    port_len = strlen(port);

    safe_free(s_agent_mqtt->bs.dmp_addr);
    safe_free(s_agent_mqtt->bs.dmp_port);

    s_agent_mqtt->bs.dmp_addr = (char *)vos_malloc(server_len + 1);
    s_agent_mqtt->bs.dmp_port = (char *)vos_malloc(port_len + 1);

    if (s_agent_mqtt->bs.dmp_addr && s_agent_mqtt->bs.dmp_port) {
        memcpy(s_agent_mqtt->bs.dmp_port, port, port_len);
        memcpy(s_agent_mqtt->bs.dmp_addr, server, server_len);
        s_agent_mqtt->bs.dmp_port[port_len] = '\0';
        s_agent_mqtt->bs.dmp_addr[server_len] = '\0';

        ///< set the flag to make the flag
        s_agent_mqtt->flag_bs_gotaddr = 1;
        s_agent_mqtt->flag_bs_run = 0; ///< we could stop it now

        ret = 0;
    } else {
        safe_free(s_agent_mqtt->bs.dmp_addr);
        safe_free(s_agent_mqtt->bs.dmp_port);
    }

EXIT_DEL_JSON_OBJ:
    cJSON_Delete(root);
EXIT_FREE_JSON_BUF:
    vos_free(json_buf);
    return ret;
}

// https://support.huaweicloud.com/api-iotps/iot_03_0004.html
static int agent_gen_bs_param(agent_mqtt_t *agent)
{
    int len;
    int i;
    unsigned char mach[CONFIG_MACH_LEN] = {0};
    char *tv = MQTT_CHECK_TIME_VALUE;
    char *pwd;

    agent_free_param(agent);

    // client id
    len = strlen(agent->config.dev_info.s.devid) + strlen(tv) +
            strlen(MQTT_OC_CLIENTID_STATIC_FMT) + 1;
    if ((agent->cli_id = (char *)vos_malloc(len)) == NULL) {
        goto EXIT_MALLOC;
    }
    snprintf(agent->cli_id, len, MQTT_OC_CLIENTID_STATIC_FMT, agent->config.dev_info.s.devid,
             agent->config.auth_type, agent->config.sign_type, tv);

    // username
    len = strlen(agent->config.dev_info.s.devid) + 1;
    if ((agent->username = (char *)vos_malloc(len)) == NULL) {
        goto EXIT_MALLOC;
    }
    memcpy(agent->username, agent->config.dev_info.s.devid, len);

    // password (hmac)
    len = sizeof(mach) * 2 + 1;
    pwd = (char *)agent->config.dev_info.s.devpwd;
    if ((agent->password = (char *)vos_malloc(len)) == NULL) {
        goto EXIT_MALLOC;
    }
    if (vsl_hmac((unsigned char *)tv, strlen(tv), (unsigned char *)pwd, strlen(pwd), mach) != 0) {
        goto EXIT_MALLOC;
    }
    for (i = 0; i < sizeof(mach); ++i) {
        // hexify, eg: 0x12 0x34 => "1234"
        snprintf(agent->password + (i << 1), 3, "%02x", mach[i]);
    }

    // subscribe topic
    len = strnlen(agent->config.dev_info.s.devid, MQTT_STRING_MAX_LEN) +
            strlen(MQTT_BS_DATA_COMMAND_FMT) + 1;
    if ((agent->sub_topic = (char *)vos_malloc(len)) == NULL) {
        goto EXIT_MALLOC;
    }
    snprintf(agent->sub_topic, len, MQTT_BS_DATA_COMMAND_FMT, agent->config.dev_info.s.devid);

    // publish topic
    len = strnlen(agent->config.dev_info.s.devid, MQTT_STRING_MAX_LEN) +
            strlen(MQTT_BS_DATA_REPORT_FMT) + 1;
    if ((agent->pub_topic = (char *)vos_malloc(len)) == NULL) {
        goto EXIT_MALLOC;
    }
    snprintf(agent->pub_topic, len, MQTT_BS_DATA_REPORT_FMT, agent->config.dev_info.s.devid);

    agent->srv_addr = (char *)agent->config.host;
    agent->srv_port = (char *)agent->config.port;
    agent->dealer = agent_handle_bs_msg;

    return 0;

EXIT_MALLOC:
    agent_free_param(agent);
    return -1;
}

static void copy_security_param(mqtt_al_scy_t *al_scy,
                                oc_mqtt_scy_t *oc_scy)
{
    if (oc_scy->type == OC_MQTT_SECURITY_PSK) {
        al_scy->type = MQTT_AL_SECURITY_PSK;
        al_scy->u.psk.id.data  = (char *)oc_scy->u.psk.psk_identity;
        al_scy->u.psk.id.len   = (int)oc_scy->u.psk.psk_identity_len;
        al_scy->u.psk.key.data = (char *)oc_scy->u.psk.psk_key;
        al_scy->u.psk.key.len  = (int)oc_scy->u.psk.psk_key_len;
    } else if (oc_scy->type == OC_MQTT_SECURTIY_CAS) {
        al_scy->type = MQTT_AL_SECURTIY_CAS;
        al_scy->u.cas.crt.data = (char *)oc_scy->u.cas.cert;
        al_scy->u.cas.crt.len  = (int)oc_scy->u.cas.cert_len;
    } else if (oc_scy->type == OC_MQTT_SECURITY_CACS) {
        al_scy->type = MQTT_AL_SECURITY_CACS;
        al_scy->u.cacs.s_crt.data = (char *)oc_scy->u.cacs.s_crt.cert;
        al_scy->u.cacs.s_crt.len  = (int)oc_scy->u.cacs.s_crt.cert_len;
        al_scy->u.cacs.c_crt.data = (char *)oc_scy->u.cacs.c_crt.cert;
        al_scy->u.cacs.c_crt.len  = (int)oc_scy->u.cacs.c_crt.cert_len;
        al_scy->u.cacs.c_key.data = (char *)oc_scy->u.cacs.c_key.cert;
        al_scy->u.cacs.c_key.len  = (int)oc_scy->u.cacs.c_key.cert_len;
        al_scy->u.cacs.host.data  = (char *)oc_scy->u.cacs.host;
        al_scy->u.cacs.host.len   = (int)strlen(oc_scy->u.cacs.host);
    }
}

// https://support.huaweicloud.com/api-iotps/iot_03_0004.html
static int agent_connect(agent_mqtt_t *agent)
{
    mqtt_al_conn_t con;

    memset(&con, 0, sizeof(mqtt_al_conn_t));

    con.clientid.data = agent->cli_id;
    con.clientid.len = strlen(agent->cli_id);

    con.username.data = agent->username;
    con.username.len = strlen(agent->username);

    con.password.data = agent->password;
    con.password.len = strlen(agent->password);

    con.cleansession = 1;
    con.keepalive = agent->config.lifetime;

    copy_security_param(&con.security, &agent->config.security);

    con.host = agent->srv_addr;
    con.port = agent->srv_port;
    con.timeout = 10000;
    con.version = MQTT_AL_VERSION_3_1_1;
    con.will = NULL;

    printf("SERVER:%s \r\n PORT:%s \r\n",\
            con.host, con.port);
    printf("CLIENTID:%s \r\n USER:%s \r\n PASSWD:%s \r\n",\
            con.clientid.data, con.username.data, con.password.data);

    return mqtt_al_connect(agent->mqtter, &con);
}

static int agent_subscribe(agent_mqtt_t *agent)
{
    mqtt_al_sub_t sub;

    memset(&sub, 0, sizeof(mqtt_al_sub_t));

    sub.qoss[0] = MQTT_AL_QOS_1;
    sub.topics[0].data = agent->sub_topic;
    sub.topics[0].len = strlen(agent->sub_topic);
    sub.count = 1;
    sub.dealer = agent->dealer;

    return mqtt_al_subscribe(agent->mqtter, &sub);
}

// https://support.huaweicloud.com/api-iotps/iot_03_0005.html
static int agent_publish(agent_mqtt_t *agent)
{
    mqtt_al_pub_t pub;

    memset(&pub, 0, sizeof(mqtt_al_pub_t));

    pub.qos = MQTT_AL_QOS_1;
    pub.retain = 0;
    pub.timeout = 1000;
    pub.topic.data = agent->pub_topic;
    pub.topic.len = strlen(agent->pub_topic);
    pub.message.data = '\0';
    pub.message.len = 0;

    return mqtt_al_publish(agent->mqtter, &pub);
}

// bs - bootstrap
static int agent_mqtt_bs(agent_mqtt_t *agent)
{
    int conn_failed_cnt = 0;

    agent->flag_bs_gotaddr = 0;

    while (agent->flag_bs_run && agent->flag_run) {
        vos_task_sleep(MQTT_CON_BACKOFF_TIME << conn_failed_cnt);
        if (agent_gen_bs_param(agent) != 0) {
            vlog_print("agent_gen_dmp_param() failed");
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        if (agent_connect(agent) != 0) {
            vlog_print("agent_connect() failed");
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        if (agent_subscribe(agent) != 0) {
            vlog_print("agent_subscribe() failed");
            mqtt_al_disconnect(agent->mqtter);
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        if (agent_publish(agent) != 0) {
            vlog_print("agent_publish() failed");
            mqtt_al_disconnect(agent->mqtter);
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }

        conn_failed_cnt = 0;
        while (agent->flag_bs_run && agent->flag_run &&
            mqtt_al_checkstatus(agent->mqtter) == MQTT_AL_ERR_NONE) {
            vos_task_sleep(1000);
        }
        mqtt_al_disconnect(agent->mqtter);
    }
    agent_free_param(agent);
    return 0;
}

// dmp - date management platform
static int agent_mqtt_dmp(agent_mqtt_t *agent)
{
    int conn_failed_cnt = 0;

    agent->flag_dmp_connected = 0;

    while (agent->flag_dmp_run && agent->flag_run) {
        vos_task_sleep(MQTT_CON_BACKOFF_TIME << conn_failed_cnt);
        if (agent_gen_dmp_param(agent) != 0) {
            vlog_print("agent_gen_dmp_param() failed");
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        if (agent_connect(agent) != 0) {
            vlog_print("agent_connect() failed");
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        if (agent_subscribe(agent) != 0) {
            vlog_print("agent_subscribe() failed");
            mqtt_al_disconnect(agent->mqtter);
            conn_failed_cnt = min(conn_failed_cnt + 1, MQTT_CON_BACKOFF_MAXTIMES);
            continue;
        }
        agent->flag_dmp_connected = 1;
        conn_failed_cnt = 0;
        while (agent->flag_dmp_run && agent->flag_run &&
            mqtt_al_checkstatus(agent->mqtter) == MQTT_AL_ERR_NONE) {
            vos_task_sleep(1000);
        }
        mqtt_al_disconnect(agent->mqtter);
        agent->flag_dmp_connected = 0;
    }
    agent_free_param(agent);
    return 0;
}

static int agent_mqtt(uintptr_t args)
{
    agent_mqtt_t *agent = (agent_mqtt_t *)args;

    while (1) {
        if (agent->flag_run) {
            agent->flag_stop = 0;

            if (agent->config.bs_mode == OC_MQTT_BS_MODE_CLIENT_INITIALIZE) {
                agent->flag_bs_run = 1;
                agent_mqtt_bs(agent);
            }
            agent->flag_dmp_run = 1;
            agent_mqtt_dmp(agent);
            agent->flag_stop = 1;
        } else {
            vos_task_sleep(1000);
        }
    }
    return 0;
}

int agent_op_init(uintptr_t *handle, oc_config_t *config)
{
    agent_mqtt_t *agent = NULL;
    int try_times = 0;

    if (s_agent_mqtt != NULL) {
        return -1;
    }
    agent = (agent_mqtt_t *)vos_zalloc(sizeof(agent_mqtt_t));
    if (agent == NULL) {
        return -1;
    }

    if (mqtt_al_init(&agent->mqtter) != 0) {
        goto EXIT;
    }

    agent->config = *config;
    agent->flag_run = 1;
    if (vos_task_create(&agent->task, "agent_mqtt", agent_mqtt, (uintptr_t)agent, 0x1800, 10) != 0) {
        goto EXIT;
    }

    s_agent_mqtt = agent;
    *handle = (uintptr_t)agent;

    while (++try_times <= 30)  {
        vos_task_sleep(MQTT_CON_BACKOFF_TIME);
        if (agent->flag_dmp_connected) {
            break;
        }
    }

    return 0;

EXIT:
    vos_free(agent);
    return -1;
}

int agent_op_destroy(uintptr_t handle)
{
    agent_mqtt_t *agent = (agent_mqtt_t *)handle;

    if (agent == NULL || agent != s_agent_mqtt) {
        return -1;
    }
    agent->flag_run = 0;
    while (!agent->flag_stop) {
        vos_task_sleep(1000);
    }

    vos_task_delete(&agent->task);
    mqtt_al_destroy(agent->mqtter);

    vos_free(agent);
    s_agent_mqtt = NULL;

    return 0;
}

int agent_op_report(uintptr_t handle, char *msg, int len, int qos)
{
    agent_mqtt_t *agent = (agent_mqtt_t *)handle;

    if (agent == NULL || agent != s_agent_mqtt || msg == NULL) {
        return -1;
    }
    if (!(agent->flag_dmp_connected && (mqtt_al_checkstatus(agent->mqtter) == MQTT_AL_ERR_NONE))) {
        return -1;
    }

    mqtt_al_pub_t pub;
    memset(&pub, 0, sizeof(mqtt_al_pub_t));

    pub.qos = qos;
    pub.retain = 0;
    pub.timeout = 1000;
    pub.topic.data = agent->pub_topic;
    pub.topic.len = strlen(agent->pub_topic);
    pub.message.data = msg;
    pub.message.len = len;

    return mqtt_al_publish(agent->mqtter, &pub);
}

static const oc_ops_t s_oc_mqtt_agent = {
    .init       = agent_op_init,
    .destroy    = agent_op_destroy,
    .report     = agent_op_report,
};

int oc_mqtt_setup(void)
{
    return oc_mqtt_register("oc_mqtt_agent", &s_oc_mqtt_agent);
}