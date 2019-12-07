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

#include "object_comm.h"


int lwm2m_add_object_ex(lwm2m_context_t *ctx, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    int ret = LWM2M_ERRNO_ERR;

    if (ctx == NULL || uri == NULL) {
        return ret;
    }
    if (uri->obj_id == LWM2M_AL_MAX_ID || uri->inst_id == LWM2M_AL_MAX_ID) {
        return ret;
    }

    switch (uri->obj_id)
    {
    case LWM2M_SECURITY_OBJECT_ID:
        ret = lwm2m_add_security_object(ctx, uri, obj_data);
        break;

    case LWM2M_SERVER_OBJECT_ID:
        ret = lwm2m_add_server_object(ctx, uri, obj_data);
        break;

    case LWM2M_ACL_OBJECT_ID:
        ret = lwm2m_add_acc_ctrl_object(ctx, uri, obj_data);
        break;

    case LWM2M_DEVICE_OBJECT_ID:
        ret = lwm2m_add_device_object(ctx, uri, obj_data);
        break;

    case LWM2M_CONN_MONITOR_OBJECT_ID:
        ret = lwm2m_add_conn_m_object(ctx, uri, obj_data);
        break;

    case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
        ret = lwm2m_add_firmware_object(ctx, uri, obj_data);
        break;

    case LWM2M_LOCATION_OBJECT_ID:
#ifdef LWM2M_CLIENT_MODE
        ret = lwm2m_add_location_object(ctx, uri, obj_data);
#endif // LWM2M_CLIENT_MODE
        break;

    case LWM2M_CONN_STATS_OBJECT_ID:
        ret = lwm2m_add_conn_s_object(ctx, uri, obj_data);
        break;

    case LWM2M_OSCORE_OBJECT_ID:
        break;

    default:
        ret = lwm2m_add_app_object(ctx, uri, obj_data);
        break;
    }

    return ret;
}

int lwm2m_check_object(lwm2m_context_t *ctx)
{
    if (ctx == NULL) {
        return LWM2M_ERRNO_INVAL;
    }

    if (ctx->objectList == NULL) {
        return LWM2M_ERRNO_NORES;
    }

    lwm2m_object_t * obj = NULL;

    obj = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, LWM2M_SECURITY_OBJECT_ID);
    if (obj == NULL) {
        return LWM2M_ERRNO_NORES;
    }
    obj = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, LWM2M_SERVER_OBJECT_ID);
    if (obj == NULL) {
        return LWM2M_ERRNO_NORES;
    }
    obj = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, LWM2M_DEVICE_OBJECT_ID);
    if (obj == NULL) {
        return LWM2M_ERRNO_NORES;
    }

    return LWM2M_ERRNO_OK;
}

int lwm2m_free_object(lwm2m_context_t *ctx)
{
    if (ctx == NULL) {
        return LWM2M_ERRNO_INVAL;
    }

    lwm2m_object_t *obj = ctx->objectList;
    while (obj != NULL) {
        lwm2m_object_t *nextP = obj->next;

        switch (obj->objID)
        {
        case LWM2M_SECURITY_OBJECT_ID:
            lwm2m_free_security_object(obj);
            break;

        case LWM2M_SERVER_OBJECT_ID:
            lwm2m_free_server_object(obj);
            break;

        case LWM2M_ACL_OBJECT_ID:
            lwm2m_free_acc_ctrl_object(obj);
            break;

        case LWM2M_DEVICE_OBJECT_ID:
            lwm2m_free_device_object(obj);
            break;

        case LWM2M_CONN_MONITOR_OBJECT_ID:
            lwm2m_free_conn_m_object(obj);
            break;

        case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
            lwm2m_free_firmware_object(obj);
            break;

        case LWM2M_LOCATION_OBJECT_ID:
#ifdef LWM2M_CLIENT_MODE
            lwm2m_free_location_object(obj);
#endif // LWM2M_CLIENT_MODE
            break;

        case LWM2M_CONN_STATS_OBJECT_ID:
            lwm2m_free_conn_s_object(obj);
            break;

        case LWM2M_OSCORE_OBJECT_ID:
            break;

        default:
            lwm2m_free_app_object(obj);
            break;
        }

        obj = nextP;
    }
    ctx->objectList = NULL;

    return LWM2M_ERRNO_OK;
}