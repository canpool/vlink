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

#ifndef __OC_LWM2M_H__
#define __OC_LWM2M_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    OC_LWM2M_BS_FACTORY = 0,
    OC_LWM2M_BS_CLIENT_INITIATED,
    OC_LWM2M_BS_SERVER_INITIATED
} oc_lwm2m_bs_e;

typedef enum {
    OC_LWM2M_MSG_APP_WRITE = 0,
    OC_LWM2M_MSG_APP_DISCOVER,
    OC_LWM2M_MSG_APP_EXECUTE,
    OC_LWM2M_MSG_SRV_REBS            // receive the rebootstrap command
} oc_lwm2m_msg_e;

typedef enum
{
    FIRMWARE_UPDATE_STATE = 0,
    APP_DATA
} oc_lwm2m_rpt_type_e;

/* security */
typedef struct {
    char           *host;
    char           *port;

    unsigned char  *psk_identity;
    size_t          psk_identity_len;
    unsigned char  *psk_key;
    size_t          psk_key_len;
} oc_lwm2m_srv_t;

typedef int (*oc_recv_fn)(void *usr_data, oc_lwm2m_msg_e type, void *msg, int len);

typedef struct {
    char           *endpoint;
    char           *manufacturer;
    char           *dev_type;

    char           *binding;
    int             lifetime;
    uint32_t        storing_cnt;
    oc_lwm2m_bs_e   bs_mode;        /* bootstrap mode  */
    int             hold_off_time;  /* bootstrap hold off time for server initiated bootstrap */

    oc_lwm2m_srv_t  server[2];      // both iot_server and bs_server have psk & pskID,
                                    // index 0 for iot_server, and index 1 for bs_server

    oc_recv_fn      recv_func;
    void           *user_data;
} oc_config_t;

typedef uintptr_t   oc_context_t;

typedef struct oc_ops {
    int (*init)     (uintptr_t *, oc_config_t *);
    int (*destroy)  (uintptr_t);
    int (*report)   (uintptr_t, int, char *, int, uint32_t);
} oc_ops_t;

int oc_lwm2m_init(oc_context_t *ctx, oc_config_t *config);
int oc_lwm2m_destroy(oc_context_t ctx);
int oc_lwm2m_report(oc_context_t ctx, int type, char *msg, int len, uint32_t timeout);

int oc_lwm2m_register(const char *name, const oc_ops_t *ops);
int oc_lwm2m_unregister(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OC_LWM2M_H__ */