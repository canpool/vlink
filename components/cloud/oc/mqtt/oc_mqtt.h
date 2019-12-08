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

#ifndef __OC_MQTT_H__
#define __OC_MQTT_H__

#include <stdint.h>
#include <stddef.h>

#include "oc_json.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    OC_MQTT_QOS_0,
    OC_MQTT_QOS_1,
    OC_MQTT_QOS_2,
    OC_MQTT_SUB_FAIL = 0x80
} oc_mqtt_qos_e;

typedef enum {
    OC_MQTT_DEV_TYPE_DYNAMIC,
    OC_MQTT_DEV_TYPE_STATIC
} oc_mqtt_dev_type_e;

typedef enum {
    OC_MQTT_CODEC_MODE_BINARY,
    OC_MQTT_CODEC_MODE_JSON,
    OC_MQTT_CODEC_MODE_MAX
} oc_mqtt_codec_mode_e;

typedef enum {
    OC_MQTT_SIGN_TYPE_HMACSHA256,
    OC_MQTT_SIGN_TYPE_HMACSHA256_CHECK_TIME
} oc_mqtt_sign_type_e;

typedef enum {
    OC_MQTT_AUTH_TYPE_DEVID,
    OC_MQTT_AUTH_TYPE_MODEL,
    OC_MQTT_AUTH_TYPE_NODEID
} oc_mqtt_auth_type_e;

typedef enum {
    OC_MQTT_BS_MODE_FACTORY,
    OC_MQTT_BS_MODE_CLIENT_INITIALIZE
} oc_mqtt_bs_mode_e;

typedef enum {
    OC_MQTT_ERR_CODE_NONE = 0,
    OC_MQTT_ERR_CODE_FAIL = 1
} oc_mqtt_err_code_e;

/* security type */
typedef enum {
    OC_MQTT_SECURITY_PSK,
    OC_MQTT_SECURTIY_CAS,
    OC_MQTT_SECURITY_CACS
} oc_mqtt_scy_e;

typedef struct {
    unsigned char  *psk_identity;
    size_t          psk_identity_len;
    unsigned char  *psk_key;
    size_t          psk_key_len;
} oc_mqtt_psk_t;

/* ca mode, only check the server */
typedef struct {
    unsigned char  *cert;
    size_t          cert_len;
} oc_mqtt_ca_t;

typedef struct {
    oc_mqtt_ca_t    s_crt; // server crt
    oc_mqtt_ca_t    c_crt; // client crt
    oc_mqtt_ca_t    c_key; // client ca key
    char           *host;
} oc_mqtt_cacs_t;

/* security config */
typedef struct {
    oc_mqtt_scy_e       type;
    union {
        oc_mqtt_psk_t   psk;
        oc_mqtt_ca_t    cas;
        oc_mqtt_cacs_t  cacs;
    } u;
} oc_mqtt_scy_t;

typedef struct {
    const char *productid;
    const char *productpwd;
    const char *noteid;
    /* operate secret */
    int       (*read)   (int offset, char *buf, int len);
    int       (*write)  (int offset, const char *buf, int len);
} oc_mqtt_dev_dynamic_t;

typedef struct {
    const char *devid;
    const char *devpwd;
} oc_mqtt_dev_static_t;

typedef struct {
    char       *topic;
    int         topic_len;
    char       *data;
    int         data_len;
    uint8_t     qos;
} oc_mqtt_rcv_t;

typedef int (*oc_mqtt_dealer)(uintptr_t, oc_mqtt_rcv_t *);

typedef struct {
    oc_mqtt_bs_mode_e   bs_mode;
    uint16_t            lifetime;
    const char         *host;
    const char         *port;
    oc_mqtt_scy_t       security;
    oc_mqtt_codec_mode_e codec_mode;
    oc_mqtt_sign_type_e sign_type;
    oc_mqtt_dev_type_e  dev_type;
    oc_mqtt_auth_type_e auth_type;
    oc_mqtt_dealer      dealer;

    union  {
        oc_mqtt_dev_dynamic_t   d;
        oc_mqtt_dev_static_t    s;
    } dev_info;
} oc_config_t;

typedef uintptr_t   oc_context_t;

typedef struct oc_ops {
    int (*init)     (uintptr_t *, oc_config_t *);
    int (*destroy)  (uintptr_t);
    int (*report)   (uintptr_t, char *, int, int);
} oc_ops_t;

int oc_mqtt_init(oc_context_t *ctx, oc_config_t *config);
int oc_mqtt_destroy(oc_context_t ctx);
int oc_mqtt_report(oc_context_t ctx, char *msg, int len, int qos);

int oc_mqtt_register(const char *name, const oc_ops_t *ops);
int oc_mqtt_unregister(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OC_MQTT_H__ */