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

#ifndef __LWM2M_AL_H__
#define __LWM2M_AL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Standard Object IDs
 */
#define LWM2M_AL_SECURITY_OBJECT_ID            0
#define LWM2M_AL_SERVER_OBJECT_ID              1
#define LWM2M_AL_ACL_OBJECT_ID                 2
#define LWM2M_AL_DEVICE_OBJECT_ID              3
#define LWM2M_AL_CONN_MONITOR_OBJECT_ID        4
#define LWM2M_AL_FIRMWARE_UPDATE_OBJECT_ID     5
#define LWM2M_AL_LOCATION_OBJECT_ID            6
#define LWM2M_AL_CONN_STATS_OBJECT_ID          7
#define LWM2M_AL_OSCORE_OBJECT_ID             21

#define LWM2M_AL_OP_READ            1
#define LWM2M_AL_OP_WRITE           2
#define LWM2M_AL_OP_EXCUTE          3
#define LWM2M_AL_OP_DISCOVER        4

typedef int (*lwm2m_al_dealer_f)(int op, const char *uri, char *msg, int len);

#define LWM2M_AL_MAX_ID     ((uint16_t)-1)

typedef struct {
    uint16_t    obj_id;
    uint16_t    inst_id;
    uint16_t    res_id;
    uint16_t    res_inst_id;
} lwm2m_al_uri_t;

typedef struct {
    char *      serverUri;
    bool        isBootstrap;
    char *      publicIdentity;
    uint16_t    publicIdLen;
    char *      secretKey;
    uint16_t    secretKeyLen;
    uint16_t    shortID;
    uint32_t    clientHoldOffTime;
} lwm2m_al_security_data_t;

typedef struct {
    uint16_t    shortServerId;
    uint32_t    lifetime;
    bool        storing;
    char *      binding;
} lwm2m_al_server_data_t;

typedef struct {
    uint16_t    storingCnt;
} lwm2m_al_app_data_t;

typedef struct {
    char *              endpoint;

    lwm2m_al_dealer_f   dealer;
} lwm2m_al_config_t;

typedef uintptr_t lwm2mer_t;

typedef struct {
    int (*init)         (uintptr_t *handle, lwm2m_al_config_t *config);
    int (*destroy)      (uintptr_t  handle);
    int (*add_object)   (uintptr_t  handle, lwm2m_al_uri_t *uri, uintptr_t obj_data);
    int (*rm_object)    (uintptr_t  handle, uint16_t obj_id);
    int (*connect)      (uintptr_t  handle);
    int (*disconnect)   (uintptr_t  handle);
    int (*send)         (uintptr_t  handle, const char *uri, const char *msg, int len, uint32_t timeout);
} lwm2m_al_ops_t;

int lwm2m_al_init(lwm2mer_t *m2m, lwm2m_al_config_t *config);
int lwm2m_al_destroy(lwm2mer_t m2m);

int lwm2m_al_add_object(lwm2mer_t m2m, lwm2m_al_uri_t *uri, uintptr_t obj_data);
int lwm2m_al_rm_object(lwm2mer_t m2m, uint16_t obj_id);

int lwm2m_al_connect(lwm2mer_t m2m);
int lwm2m_al_disconnect(lwm2mer_t m2m);

int lwm2m_al_send(lwm2mer_t m2m, const char *uri, const char *msg, int len, uint32_t timeout);

int lwm2m_al_install(const lwm2m_al_ops_t *ops);
int lwm2m_al_uninstall(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LWM2M_AL_H__ */