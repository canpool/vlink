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

#ifndef __MQTT_AL_H__
#define __MQTT_AL_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CONFIG_MAX_SUB_TOPIC     5

typedef enum {
    MQTT_AL_VERSION_3_1_0 = 3,
    MQTT_AL_VERSION_3_1_1 = 4
} mqtt_al_ver_e;

typedef enum {
    MQTT_AL_QOS_0,
    MQTT_AL_QOS_1,
    MQTT_AL_QOS_2,
    MQTT_AL_SUB_FAIL = 0x80
} mqtt_al_qos_e;

typedef enum {
    MQTT_AL_ERR_NONE,
    MQTT_AL_ERR_INVAL,
    MQTT_AL_ERR_CONN_FAIL
} mqtt_al_err_e;

/* security type */
typedef enum {
    MQTT_AL_SECURITY_PSK,
    MQTT_AL_SECURTIY_CAS,
    MQTT_AL_SECURITY_CACS
} mqtt_al_scy_e;

typedef struct {
    int   len;
    char *data;
} mqtt_al_string_t;

typedef struct {
    mqtt_al_string_t    id;
    mqtt_al_string_t    key;
} mqtt_al_psk_t;

/* ca mode, only check the server */
typedef struct {
    mqtt_al_string_t    crt;
} mqtt_al_cas_t;

typedef struct {
    mqtt_al_string_t    s_crt; // server crt
    mqtt_al_string_t    c_crt; // client crt
    mqtt_al_string_t    c_key; // client ca key
    mqtt_al_string_t    host;
} mqtt_al_cacs_t;

/* security config */
typedef struct {
    mqtt_al_scy_e       type;
    union {
        mqtt_al_psk_t   psk;
        mqtt_al_cas_t   cas;
        mqtt_al_cacs_t  cacs;
    } u;
} mqtt_al_scy_t;

/* will options */
typedef struct {
    /* connect flag bits */
    uint8_t             qos;
    uint8_t             retain;
    /* payload */
    mqtt_al_string_t    topic;
    mqtt_al_string_t    message;
} mqtt_al_will_t;

/* connect data */
typedef struct {
    char               *host;
    char               *port;
    mqtt_al_scy_t      *security;

    mqtt_al_ver_e       version;
    mqtt_al_string_t    clientid;
    uint16_t            keepalive;
    uint8_t             cleansession;
    mqtt_al_will_t     *will;
    mqtt_al_string_t    username;
    mqtt_al_string_t    password;
    uint32_t            timeout;
} mqtt_al_conn_t;

/* publish message */
typedef struct {
    uint8_t             retain;
    uint8_t             qos;
    uint8_t             dup;
    mqtt_al_string_t    topic;
    uint16_t            packetid;   /**< used when the QoS level is 1 or 2 */
    mqtt_al_string_t    message;
    uint32_t            timeout;
} mqtt_al_pub_t;

typedef struct {
	mqtt_al_string_t    topic;
	mqtt_al_string_t    msg;
	uint8_t             qos;
	uint8_t             dup;
	uint8_t             retain;
} mqtt_al_rcv_t;

typedef int (*mqtt_al_dealer)(uintptr_t, mqtt_al_rcv_t *);

/* subscribe topic */
typedef struct {
    uint8_t             dup;
    uint16_t            packetid;
    mqtt_al_string_t    topics[CONFIG_MAX_SUB_TOPIC];   // topic filter
    uint8_t             qoss[CONFIG_MAX_SUB_TOPIC];     // requested qos
    int                 count;      // topic filter count
    mqtt_al_dealer      dealer;
} mqtt_al_sub_t;

typedef struct {
    uint16_t            packetid;
    uint8_t             qoss[CONFIG_MAX_SUB_TOPIC];     // granted qos
    int                 count;
} nqtt_al_suback_t;

typedef struct {
    mqtt_al_string_t    topics[CONFIG_MAX_SUB_TOPIC];   // topic filter
    int                 count;
} mqtt_al_unsub_t;

typedef uintptr_t       mqtter_t;

typedef struct {
    int (*init)         (uintptr_t *handle);
    int (*destroy)      (uintptr_t  handle);
    int (*connect)      (uintptr_t  handle, mqtt_al_conn_t *con);
    int (*disconnect)   (uintptr_t  handle);
    int (*publish)      (uintptr_t  handle, mqtt_al_pub_t *pub);
    int (*subscribe)    (uintptr_t  handle, mqtt_al_sub_t *sub);
    int (*unsubscribe)  (uintptr_t  handle, mqtt_al_unsub_t *unsub);
    int (*checkstatus)  (uintptr_t  handle);
} mqtt_al_ops_t;

int mqtt_al_init(mqtter_t *mqtter);
int mqtt_al_destroy(mqtter_t mqtter);
int mqtt_al_connect(mqtter_t mqtter, mqtt_al_conn_t *con);
int mqtt_al_disconnect(mqtter_t mqtter);
int mqtt_al_publish(mqtter_t mqtter, mqtt_al_pub_t *pub);
int mqtt_al_subscribe(mqtter_t mqtter, mqtt_al_sub_t *sub);
int mqtt_al_unsubscribe(mqtter_t mqtter, mqtt_al_unsub_t *unsub);
int mqtt_al_checkstatus(mqtter_t mqtter);

int mqtt_al_install(const mqtt_al_ops_t *ops);
int mqtt_al_uninstall(void);

int mqtt_al_checkfilter(const mqtt_al_string_t *topic_filter);
int mqtt_al_checkname(const mqtt_al_string_t *topic_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MQTT_AL_H__ */