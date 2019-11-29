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

#include "mqtt_port.h"

#include "MQTTClient.h"

#include <string.h>

#include "mqtt_al.h"
#include "sal.h"
#include "vos.h"

#ifdef WITH_DTLS
#include "dtls.h"
#endif

#define MQTT_CMD_TIMEOUT_MS (10 * 1000)
#define MQTT_SEND_BUF_SIZE  (1024 * 2)
#define MQTT_RECV_BUF_SIZE  (1024 * 2)

typedef struct {
    Network              network;
    MQTTClient           client;
    vtask_t              task;
    void                *sendbuf;
    void                *recvbuf;
} mqtt_cb_t;

static int s_yield = 0;

#ifdef WITH_DTLS
static int __tls_connect(Network *n, const char *host, const char *port)
{
    int ret;
    dtls_config_info config_info;
    dtls_handshake_info handshake_info;
    dtls_context *context = NULL;
    mqtt_al_scy_t *security = (mqtt_al_scy_t *)n->arg;

    if (security == NULL) {
        return -1;
    }

    memset(&config_info, 0, sizeof(dtls_config_info));
    memset(&handshake_info, 0, sizeof(dtls_handshake_info));

    config_info.proto = MBEDTLS_NET_PROTO_TCP;
    config_info.endpoint = MBEDTLS_SSL_IS_CLIENT;
    config_info.pers = "mqtt_dtls_client";
    if (security->type == MQTT_AL_SECURITY_PSK) {
        config_info.auth_type |= DTLS_AUTH_PSK;
        config_info.psk.psk_key = (unsigned char *)security->u.psk.key.data;
        config_info.psk.psk_key_len = security->u.psk.key.len;
        config_info.psk.psk_identity = (unsigned char *)security->u.psk.id.data;
        config_info.psk.psk_identity_len = security->u.psk.id.len;
    } else if (security->type == MQTT_AL_SECURTIY_CAS) {
        config_info.auth_type |= DTLS_AUTH_CA;
        config_info.ca.ca_cert = (unsigned char *)security->u.cas.crt.data;
        config_info.ca.ca_cert_len = security->u.cas.crt.len;
    } else {
        return -1;
    }

    if ((context = dtls_create(&config_info)) == NULL) {
        return -1;
    }
    if ((ret = dtls_connect(context, host, port)) != 0) {
        goto EXIT_FAIL;
    }
    handshake_info.timeout = CONFIG_DTLS_HANDSHAKE_TIMEOUT;
    if ((ret = dtls_handshake(context, &handshake_info)) != 0) {
        goto EXIT_FAIL;
    }
    n->ctx = (uintptr_t)context;
    return 0;

EXIT_FAIL:
    dtls_destroy(context);
    return -1;
}

static int __tls_disconnect(uintptr_t ctx)
{
    dtls_context *context = (dtls_context *)ctx;

    if (context == NULL) {
        return -1;
    }
    dtls_destroy(context);
    return 0;
}
#else
static int __socket_connect(Network *n, const char *host, const char *port)
{
    int fd = sal_sockconnect(AF_INET, SOCK_STREAM, 0, host, port);

    if (fd < 0) {
        return -1;
    }
    n->ctx = (uintptr_t)fd;
    return 0;
}

static int __socket_disconnect(uintptr_t ctx)
{
    return sal_closesocket((int)ctx);
}
#endif // WITH_DTLS

static int __io_read(Network *n, unsigned char *buf, int len, int timeout)
{
    int ret;

    if (n == NULL || buf == NULL) {
        return -1;
    }
#ifdef WITH_DTLS
    ret = dtls_read((dtls_context *)n->ctx, (char *)buf, len, timeout);
#else
    ret = sal_timedrecv((int)n->ctx, (const char *)buf, len, 0, timeout);
#endif
    if (ret == 0) {
        ret = -1;
    } else if (ret < 0) {
        ret = 0;
    }
    return ret;
}

static int __io_write(Network *n, unsigned char *buf, int len, int timeout)
{
    int ret;

    if (n == NULL || buf == NULL) {
        return -1;
    }
#ifdef WITH_DTLS
    ret = dtls_write((dtls_context *)n->ctx, (char *)buf, len);
#else
    ret = sal_timedsend((int)n->ctx, (const char *)buf, len, 0, timeout);
#endif
    if (ret == 0) {
        ret = -1;
    } else if (ret < 0) {
        ret = 0;
    }
    return ret;
}

static int __io_connect(Network *n, const char *host, const char *port)
{
#ifdef WITH_DTLS
    if (n->arg != 0) {
        return __tls_connect(n, host, port);
    }
    return -1;
#else
    return __socket_connect(n, host, port);
#endif
}

static int __io_disconnect(Network *n)
{
    int ret;
#ifdef WITH_DTLS
    ret = __tls_disconnect(n->ctx);
#else
    ret = __socket_disconnect(n->ctx);
#endif
    n->ctx = 0;
    return ret;
}

static int __task_entry(uintptr_t arg)
{
    mqtt_cb_t *cb = (mqtt_cb_t *)arg;

    while (1) {
        // poll here, if you need
        if ((cb != NULL) && MQTTIsConnected(&cb->client) && s_yield) {
            s_yield = 0;
            MQTTYield(&cb->client, 10);
        } else {
            vos_task_sleep(10);
        }
    }
    return 0;
}

static int __init(uintptr_t *handle)
{
    mqtt_cb_t   *cb = NULL;
    Network     *n = NULL;
    MQTTClient  *c = NULL;

    cb = (mqtt_cb_t *)vos_zalloc(sizeof(mqtt_cb_t));
    if (cb == NULL) {
        return -1;
    }

    n = &cb->network;
    n->mqttread = __io_read;
    n->mqttwrite = __io_write;

    cb->sendbuf = vos_zalloc(MQTT_SEND_BUF_SIZE);
    cb->recvbuf = vos_zalloc(MQTT_RECV_BUF_SIZE);
    if (cb->sendbuf == NULL || cb->recvbuf == NULL) {
        goto EXIT_BUF_MEM_ERR;
    }

    c = &cb->client;
    MQTTClientInit(c, n, MQTT_CMD_TIMEOUT_MS, cb->sendbuf, MQTT_SEND_BUF_SIZE, cb->recvbuf, MQTT_RECV_BUF_SIZE);

    *handle = (uintptr_t)cb;
    return 0;

EXIT_BUF_MEM_ERR:
    if (cb->sendbuf) {
        vos_free(cb->sendbuf);
    }
    if (cb->recvbuf) {
        vos_free(cb->recvbuf);
    }
    vos_free(cb);
    return -1;
}

static int __destroy(uintptr_t handle)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    Network     *n = &cb->network;
    MQTTClient  *c = &cb->client;

    if (MQTTIsConnected(c)) {
        MQTTDisconnect(c);
        __io_disconnect(n);
        vos_task_delete(&cb->task);
    }

    MQTTClientDeInit(c);
    vos_free(cb->sendbuf);
    vos_free(cb->recvbuf);
    vos_free(cb);

    return 0;
}

static int __connect(uintptr_t handle, mqtt_al_conn_t *con)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    Network     *n = &cb->network;
    MQTTClient  *c = &cb->client;
    MQTTPacket_connectData option = MQTTPacket_connectData_initializer;
    MQTTConnackData conack;
    int ret;

    n->arg = (uintptr_t)con->security;

    if ((ret = __io_connect(n, con->host, con->port)) != 0) {
        vlog_print("__io_connect() failed, ret = %d", ret);
        return -1;
    }
    option.MQTTVersion = con->version;

    option.clientID.lenstring.len = con->clientid.len;
    option.clientID.lenstring.data = con->clientid.data;

    option.keepAliveInterval = con->keepalive;
    option.cleansession = con->cleansession;

    if (con->will != NULL) {
        option.willFlag = 1;
        option.will.qos = con->will->qos;
        option.will.retained = con->will->retain;

        option.will.topicName.lenstring.len = con->will->topic.len;
        option.will.topicName.lenstring.data = con->will->topic.data;

        option.will.message.lenstring.len = con->will->message.len;
        option.will.message.lenstring.data = con->will->message.data;
    } else {
        option.willFlag = 0;
    }

    option.username.lenstring.len = con->username.len;
    option.username.lenstring.data = con->username.data;

    option.password.lenstring.len = con->password.len;
    option.password.lenstring.data = con->password.data;

    if ((ret = MQTTConnectWithResults(c, &option, &conack)) != SUCCESS || conack.rc != 0) {
        vlog_print("MQTTConnectWithResults() failed, ret = %d", ret);
        goto EXIT_MQTT_CONNECT;
    }

    if (vos_task_create(&cb->task, "paho", __task_entry, (uintptr_t)cb, 0x800, 4) != 0) {
        goto EXIT_TASK_ERR;
    }

    return 0;

EXIT_TASK_ERR:
    MQTTDisconnect(c);
EXIT_MQTT_CONNECT:
    __io_disconnect(n);
    return -1;
}

static int __disconnect(uintptr_t handle)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    Network     *n = &cb->network;
    MQTTClient  *c = &cb->client;

    MQTTDisconnect(c);
    __io_disconnect(n);
    vos_task_delete(&cb->task);

    return 0;
}

static int __publish(uintptr_t handle, mqtt_al_pub_t *pub)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    MQTTClient  *c = &cb->client;
    MQTTMessage  msg;
    int ret;

    memset(&msg, 0, sizeof(MQTTMessage));
    msg.retained = pub->retain;
    msg.qos = pub->qos;
    msg.payload = pub->message.data;
    msg.payloadlen = pub->message.len;

    // set yield flag
    s_yield = (msg.payloadlen == 0 ? 1 : 0);

    if ((ret = MQTTPublish(c, pub->topic.data, &msg)) != SUCCESS) {
        vlog_print("MQTTPublish() failed, ret = %d", ret);
    }

    return ret;
}

static void message_handler(MessageData *data)
{
    mqtt_al_rcv_t   rcvmsg;
    mqtt_al_dealer  dealer;

    rcvmsg.dup = data->message->dup;
    rcvmsg.qos = data->message->qos;
    rcvmsg.retain = data->message->retained;
    rcvmsg.msg.len = data->message->payloadlen;
    rcvmsg.msg.data = data->message->payload;

    if (data->topicName->lenstring.len) {
        rcvmsg.topic.data = data->topicName->lenstring.data;
        rcvmsg.topic.len = data->topicName->lenstring.len;
    } else {
        rcvmsg.topic.data = data->topicName->cstring;
        rcvmsg.topic.len = strlen(data->topicName->cstring);
    }

    if (data->arg != NULL) {
        dealer = (mqtt_al_dealer)data->arg;
        dealer(0, &rcvmsg);
    }
}

static int __subscribe(uintptr_t handle, mqtt_al_sub_t *sub)
{
    mqtt_cb_t       *cb = (mqtt_cb_t *)handle;
    MQTTClient      *c = &cb->client;
    MQTTSubackData   ack;
    int ret;

    if ((ret = MQTTSubscribeWithResults(c, sub->topics[0].data, sub->qoss[0],
            message_handler, &ack, sub->dealer)) != SUCCESS) {
        vlog_print("MQTTSubscribeWithResults() failed, ret = %d", ret);
    }

    return ret;
}

static int __unsubscribe(uintptr_t handle, mqtt_al_unsub_t *unsub)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    MQTTClient  *c = &cb->client;
    int i;

    for (i = 0; i < unsub->count; ++i) {
        if (MQTTUnsubscribe(c, unsub->topics[i].data) != SUCCESS) {
            return -1;
        }
    }

    return 0;
}

static int __checkstatus(uintptr_t handle)
{
    mqtt_cb_t   *cb = (mqtt_cb_t *)handle;
    MQTTClient  *c = &cb->client;

    if (MQTTIsConnected(c)) {
        return MQTT_AL_ERR_NONE;
    }
    return MQTT_AL_ERR_CONN_FAIL;
}

static mqtt_al_ops_t s_paho_mqtt_ops = {
    .init = __init,
    .destroy = __destroy,
    .connect = __connect,
    .disconnect = __disconnect,
    .publish = __publish,
    .subscribe = __subscribe,
    .unsubscribe = __unsubscribe,
    .checkstatus = __checkstatus,
};

int mqtt_setup(void)
{
    return mqtt_al_install(&s_paho_mqtt_ops);
}