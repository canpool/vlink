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

#include <ctype.h>

#include "lwm2m_rpt.h"

#define MAX_RES_NUM         32
#define MAX_STRURI_LEN      20

#define MIN_SAVE_CNT        2

#define URI_FMT             "/%u/%u/%u"

typedef struct _app_instance_ {
    struct _app_instance_ * next;
    uint16_t                shortID;
    uint16_t                objID;
    uint32_t                resourceMap;
    rpt_list_t *            rptlst;
} app_instance_t;

static int prv_new_data(app_instance_t *targetP,
                            int *numDataP,
                            lwm2m_data_t **dataArrayP)
{
    uint32_t resMap = targetP->resourceMap;
    uint32_t resCnt = 0;
    uint16_t resId = 0;
    int i = 0;

    while (resMap)  {
        if (resMap & 0x1) {
            resCnt++;
        }
        resMap >>= 1;
    }

    if (resCnt == 0) {
        return -1;
    }

    *dataArrayP = lwm2m_data_new(resCnt);
    if (*dataArrayP == NULL) {
        return -1;
    }
    *numDataP = resCnt;

    resMap = targetP->resourceMap;
    while (resMap && i < resCnt)  {
        if (resMap & 0x1) {
            (*dataArrayP)[i].id = resId;
            i++;
        }
        resMap >>= 1;
        resId++;
    }

    return 0;
}

static uint8_t prv_read_data(app_instance_t *targetP,
                             int numData,
                             lwm2m_data_t *dataArrayP)
{
    int i;
    rpt_data_t data;
    int ret;

    for (i = 0; i < numData; i++) {
        if (targetP->resourceMap & (1 << dataArrayP[i].id)) {
            ret = lwm2m_rpt_dequeue_data(targetP->rptlst, &data);
            if (ret != 0) {
                return COAP_404_NOT_FOUND;
            }
            dataArrayP[i].type = LWM2M_TYPE_OPAQUE;
            dataArrayP[i].value.asBuffer.buffer = data.buf;
            dataArrayP[i].value.asBuffer.length = data.len;
        } else {
            return COAP_404_NOT_FOUND;
        }
    }

    return COAP_205_CONTENT;
}

static void prv_destroy_data_buf(app_instance_t *targetP,
                                 int numData,
                                 lwm2m_data_t *dataArrayP)
{
    int i;
    for (i = 0; i < numData; i++) {
        if (targetP->resourceMap & (1 << dataArrayP[i].id)) {
            if (dataArrayP[i].value.asBuffer.buffer != NULL) {
                lwm2m_free(dataArrayP[i].value.asBuffer.buffer);
                dataArrayP[i].value.asBuffer.buffer = NULL;
                dataArrayP[i].value.asBuffer.length = 0;
            }
        }
    }
}

static uint8_t prv_read(uint16_t instanceId,
                        int *numDataP,
                        lwm2m_data_t **dataArrayP,
                        lwm2m_object_t *objectP)
{
    app_instance_t *targetP;
    uint8_t ret;
    bool new_data_flag = false;

    targetP = (app_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) {
        return COAP_404_NOT_FOUND;
    }

    if (*numDataP == 0) {
        if (prv_new_data(targetP, numDataP, dataArrayP) != 0) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        new_data_flag = true;
    }

    ret = prv_read_data(targetP, *numDataP, *dataArrayP);
    if (ret != COAP_205_CONTENT) {
        prv_destroy_data_buf(targetP, *numDataP, *dataArrayP);
        if (new_data_flag) {
            lwm2m_free(*dataArrayP);
            *dataArrayP = NULL;
        }
    }

    return ret;
}

static uint8_t prv_discover(uint16_t instanceId,
                            int *numDataP,
                            lwm2m_data_t **dataArrayP,
                            lwm2m_object_t *objectP)
{
    int i;
    app_instance_t *targetP;

    targetP = (app_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0) {
        if (prv_new_data(targetP, numDataP, dataArrayP) != 0) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    } else {
        for (i = 0; i < *numDataP; i++) {
            if (!(targetP->resourceMap & (1 << (*dataArrayP)[i].id))) {
                return COAP_404_NOT_FOUND;
            }
        }
    }

    return COAP_205_CONTENT;
}

static uint8_t prv_write(uint16_t instanceId,
                         int numData,
                         lwm2m_data_t *dataArray,
                         lwm2m_object_t *objectP)
{
    app_instance_t *targetP;
    int i;
    char uri[MAX_STRURI_LEN] = {0};

    targetP = (app_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP)
        return COAP_404_NOT_FOUND;

    for (i = 0; i < numData; i++) {
        if (targetP->resourceMap & (1 << dataArray[i].id)) {
            snprintf(uri, MAX_STRURI_LEN, URI_FMT, targetP->objID, instanceId, dataArray[i].id);
            lwm2m_cmd_ioctl(LWM2M_CMD_MAX,
                            (char *)(dataArray[i].value.asBuffer.buffer),
                            dataArray->value.asBuffer.length,
                            LWM2M_AL_OP_WRITE,
                            uri);
        } else {
            return COAP_404_NOT_FOUND;
        }
    }

    return COAP_204_CHANGED;
}

static void prv_rm_rpt(app_instance_t *targetP)
{
    lwm2m_uri_t uri;
    uint32_t resMap = targetP->resourceMap;
    uint16_t resId = 0;

    while (resMap)  {
        if (resMap & 0x1) {
            lwm2m_uri_res(&uri, targetP->objID, targetP->shortID, resId);
            lwm2m_rpt_rm_uri(&uri);
        }
        resMap >>= 1;
        resId++;
    }
}

static uint8_t prv_delete(uint16_t id, lwm2m_object_t *objectP)
{
    app_instance_t *targetP;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&targetP);
    if (NULL == targetP)
        return COAP_404_NOT_FOUND;

    prv_rm_rpt(targetP);
    lwm2m_free(targetP);

    return COAP_202_DELETED;
}

static uint8_t prv_create(uint16_t instanceId,
                         int numData,
                         lwm2m_data_t *dataArray,
                         lwm2m_object_t *objectP)
{
    app_instance_t *targetP;
    uint8_t result;
    lwm2m_uri_t uri;
    int i;

    if (instanceId == LWM2M_MAX_ID) {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    targetP = (app_instance_t *)LWM2M_LIST_FIND(objectP->instanceList, instanceId);
    if (targetP != NULL)
        return COAP_500_INTERNAL_SERVER_ERROR;

    // check resource id
    for (i = 0; i < numData; ++i) {
        if (dataArray[i].id >= MAX_RES_NUM) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    targetP = (app_instance_t *)lwm2m_malloc(sizeof(app_instance_t));
    if (targetP == NULL) {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
    memset(targetP, 0, sizeof(app_instance_t));

    targetP->shortID = instanceId;
    targetP->objID = objectP->objID;

    // add rpt
    for (i = 0; i < numData; ++i) {
        targetP->resourceMap |= dataArray[i].id;

        lwm2m_uri_res(&uri, objectP->objID, instanceId, dataArray[i].id);
        targetP->rptlst = lwm2m_rpt_add_uri(&uri);
        if (targetP->rptlst == NULL) {
            lwm2m_free(targetP);
            return COAP_404_NOT_FOUND;
        }
    }
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, targetP);

    result = prv_write(instanceId, numData, dataArray, objectP);

    if (result != COAP_204_CHANGED) {
        prv_delete(instanceId, objectP);
    } else {
        result = COAP_201_CREATED;
    }

    return result;
}

static uint8_t prv_exec(uint16_t instanceId,
                        uint16_t resourceId,
                        uint8_t *buffer,
                        int length,
                        lwm2m_object_t *objectP)
{
    app_instance_t *targetP;
    char uri[MAX_STRURI_LEN] = {0};

    targetP = (app_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (targetP == NULL)
        return COAP_404_NOT_FOUND;

    if (targetP->resourceMap & (1 << resourceId)) {
        snprintf(uri, MAX_STRURI_LEN, URI_FMT, targetP->objID, instanceId, resourceId);
        lwm2m_cmd_ioctl(LWM2M_CMD_MAX,
                        (char *)buffer,
                        length,
                        LWM2M_AL_OP_EXCUTE,
                        uri);
    } else {
        return COAP_404_NOT_FOUND;
    }

    return COAP_204_CHANGED;
}

void display_app_object(lwm2m_object_t *object)
{
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Test object, instances:\r\n", object->objID);
    app_instance_t *instance = (app_instance_t *)object->instanceList;
    while (instance != NULL) {
        fprintf(stdout, "    /%u/%u: resourceMap: %32x\r\n",
                object->objID, instance->shortID, instance->resourceMap);
        instance = instance->next;
    }
#endif
}

static void free_app_object_rpt(lwm2m_object_t *object)
{
    app_instance_t *cur =
        (app_instance_t *)object->instanceList;

    while (cur) {
        prv_rm_rpt(cur);
        cur = cur->next;
    }
}

void free_app_object(lwm2m_object_t *object)
{
    free_app_object_rpt(object);
    LWM2M_LIST_FREE(object->instanceList);
    ///< for this memory is not allocated by us, we should not take care about it
//    if (object->userData != NULL)
//    {
//        lwm2m_free(object->userData);
//        object->userData = NULL;
//    }
    lwm2m_free(object);
}

static lwm2m_object_t *lwm2m_get_app_object(uint16_t obj_id)
{
    lwm2m_object_t * appObj;

    appObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    if (appObj == NULL) {
        return NULL;
    }

    memset(appObj, 0, sizeof(lwm2m_object_t));
    appObj->objID = obj_id;

    appObj->readFunc = prv_read;
    appObj->discoverFunc = prv_discover;
    appObj->writeFunc = prv_write;
    appObj->executeFunc = prv_exec;
    appObj->createFunc = prv_create;
    appObj->deleteFunc = prv_delete;

    return appObj;
}

static int lwm2m_add_app_instance(lwm2m_object_t *obj, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    int ret = LWM2M_ERRNO_OK;

    lwm2m_al_app_data_t *dataP = (lwm2m_al_app_data_t *)obj_data;
    if (dataP == NULL) {
        return LWM2M_ERRNO_INVAL;
    }

    app_instance_t * targetP;
    lwm2m_uri_t uri_tmp;

    targetP = (app_instance_t *)lwm2m_malloc(sizeof(app_instance_t));
    if (targetP == NULL) {
        return LWM2M_ERRNO_NOMEM;
    }
    memset(targetP, 0, sizeof(app_instance_t));

    targetP->shortID = uri->inst_id;
    targetP->objID = uri->obj_id;
    targetP->resourceMap |= (1 << uri->res_id);

    lwm2m_uri_res(&uri_tmp, uri->obj_id, uri->inst_id, uri->res_id);
    targetP->rptlst = lwm2m_rpt_add_uri(&uri_tmp);
    if (targetP->rptlst == NULL) {
        lwm2m_free(targetP);
        return LWM2M_ERRNO_NORES;
    }
    lwm2m_rpt_set_max_cnt(&uri_tmp, MAX(MIN_SAVE_CNT, dataP->storingCnt));

    obj->instanceList = LWM2M_LIST_ADD(obj->instanceList, targetP);

    return ret;
}

int lwm2m_add_app_object(lwm2m_context_t *ctx, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    int ret = LWM2M_ERRNO_OK;
    bool is_new = false;

    if (uri->res_id >= MAX_RES_NUM) {
        return LWM2M_ERRNO_NORES;
    }

    lwm2m_object_t * obj = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, uri->obj_id);
    if (obj == NULL) {
        obj = lwm2m_get_app_object(uri->obj_id);
        if (obj == NULL) {
            return LWM2M_ERRNO_NOMEM;
        }
        is_new = true;
    }

    app_instance_t *inst = (app_instance_t *)LWM2M_LIST_FIND(obj->instanceList, uri->inst_id);
    if (inst == NULL) {
        ret = lwm2m_add_app_instance(obj, uri, obj_data);
        if (ret != LWM2M_ERRNO_OK) {
            if (is_new) {
                lwm2m_free(obj);
            }
            return ret;
        }
    } else {
        uint32_t flag = 1 << uri->res_id;
        if (inst->resourceMap & flag) {
            return LWM2M_ERRNO_REPEAT;
        }
        inst->resourceMap |= flag;
    }

    if (is_new) {
        lwm2m_add_object(ctx, obj);
    }

    return ret;
}