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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    LWM2M_AL_BS_FACTORY = 0,
    LWM2M_AL_BS_CLIENT_INITIATED,
    LWM2M_AL_BS_SERVER_INITIATED
} lwm2m_al_bs_e;

typedef struct {
    char           *host;
    char           *port;

    unsigned char  *psk_identity;
    size_t          psk_identity_len;
    unsigned char  *psk_key;
    size_t          psk_key_len;
} lwm2m_al_srv_t;

typedef struct {
    char           *endpoint;
    char           *manufacturer;
    char           *dev_type;

    char           *binding;
    int             lifetime;
    uint32_t        storing_cnt;
    lwm2m_al_bs_e   bs_mode;        /* bootstrap mode  */
    int             hold_off_time;  /* bootstrap hold off time for server initiated bootstrap */

    lwm2m_al_srv_t  server[2];      // both iot_server and bs_server have psk & pskID,
                                    // index 0 for iot_server, and index 1 for bs_server
    void           *usr_data;
} lwm2m_al_config_t;

typedef uintptr_t lwm2mer_t;

typedef struct {
    int (*init)     (uintptr_t *handle, lwm2m_al_config_t *config);
    int (*destroy)  (uintptr_t  handle);
    int (*send)     (uintptr_t  handle, const char *uri, const char *msg, int len, uint32_t timeout);
} lwm2m_al_ops_t;

int lwm2m_al_init(lwm2mer_t *m2m, lwm2m_al_config_t *config);
int lwm2m_al_destroy(lwm2mer_t m2m);

// Valid URIs: /1, /1/, /1/2, /1/2/, /1/2/3
// Invalid URIs: /, //, //2, /1//, /1//3, /1/2/3/, /1/2/3/4
int lwm2m_al_send(lwm2mer_t m2m, const char *uri, const char *msg, int len, uint32_t timeout);

int lwm2m_al_install(const lwm2m_al_ops_t *ops);
int lwm2m_al_uninstall(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LWM2M_AL_H__ */