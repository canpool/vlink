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

#ifndef __LWM2M_PORT_H__
#define __LWM2M_PORT_H__

#include "liblwm2m.h"
#include "object_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#define SERVER_URI_MAX_LEN  (64)
#define MAX_PACKET_SIZE     (1024)
#define SERVER_ID           (123)
#define BIND_TIMEOUT        (10)

enum {
    OBJ_SECURITY_INDEX = 0,
    OBJ_SERVER_INDEX,
    OBJ_ACCESS_CONTROL_INDEX,
    OBJ_DEVICE_INDEX,
    OBJ_CONNECT_INDEX,
    OBJ_FIRMWARE_INDEX,
    OBJ_LOCATION_INDEX,
    OBJ_APP_INDEX,
    OBJ_MAX_NUM,
};


int lwm2m_setup(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LWM2M_PORT_H__ */