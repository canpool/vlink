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
#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <liblwm2m.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LWM2M_STANDARD_PORT_STR "5683"
#define LWM2M_STANDARD_PORT      5683
#define LWM2M_DTLS_PORT_STR     "5684"
#define LWM2M_DTLS_PORT          5684
#define LWM2M_BSSERVER_PORT_STR "5685"
#define LWM2M_BSSERVER_PORT      5685

typedef struct _connection_t
{
    struct _connection_t    *next;
    uintptr_t                sock;
    lwm2m_object_t          *securityObj;
    int                      securityInstId;
    lwm2m_context_t         *lwm2mH;
    uint8_t                  dtls_flag;
} connection_t;

int lwm2m_buffer_recv(void *sessionH, uint8_t *buffer, size_t length, uint32_t timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __CONNECTION_H__ */
