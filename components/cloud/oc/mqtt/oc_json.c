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

#include "oc_json.h"
#include "oc_mqtt.h"

static cJSON *oc_json_fmt_value(oc_json_t *disc)
{
    cJSON *ret = NULL;

    switch (disc->type) {
    case OC_JSON_TYPE_INT:
        ret = cJSON_CreateNumber((double)(*(int *)disc->value));
        break;
    case OC_JSON_TYPE_STRING:
        ret = cJSON_CreateString((const char *)disc->value);
        break;
    default:
        break;
    }

    return ret;
}

cJSON *oc_json_fmt_report(oc_json_report_t *report)
{
    cJSON *service_data = NULL;
    cJSON *data_array = NULL;
    cJSON *service = NULL;
    cJSON *root;
    cJSON *tmp;

    oc_json_t *kvlst;

    ///< create the root object
    root = cJSON_CreateObject();
    if (root == NULL) {
        goto EXIT_CJSON_ERR;
    }
    ///< create the msg type object and add it to the root
    tmp = cJSON_CreateString(OC_JSON_VAL_DeviceReq);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_MsgType, tmp);
    ///< create the has more item and add it to the root
    tmp = cJSON_CreateNumber((double)report->hasmore);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_HasMore, tmp);
    ///< create the data array and add it to the root
    data_array = cJSON_CreateArray();
    if (data_array == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_Data, data_array);

    ///< create the service and add it to the data_array
    service = cJSON_CreateObject();
    if (service == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToArray(data_array, service);

    ///< create the service id and add to the service
    tmp = cJSON_CreateString(report->srvid);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(service, OC_JSON_KEY_ServiceId, tmp);

    ///< create the service_data object and add it  to the service
    service_data = cJSON_CreateObject();
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(service, OC_JSON_KEY_ServiceData, service_data);

    ///< loop the report data and make the corresponding k-v object,then add it to the service_data
    kvlst = report->paralst;
    while (kvlst != NULL) {
        ///< fmt the value to a json object
        tmp = oc_json_fmt_value(kvlst);
        if (tmp == NULL) {
            goto EXIT_CJSON_ERR;
        }
        ///< add the object to the serice data
        cJSON_AddItemToObject(service_data, kvlst->key, tmp);

        kvlst = kvlst->next;
    }

    ///< create the time service_data object and add it to the service
    if (report->eventtime != NULL) {
        tmp = cJSON_CreateString(report->eventtime);
        if (tmp == NULL) {
            goto EXIT_CJSON_ERR;
        }
        cJSON_AddItemToObject(service, OC_JSON_KEY_EventTime, tmp);
    }

    return root;

EXIT_CJSON_ERR:
    if (root) {
        cJSON_Delete(root);
    }

    return NULL;
}

cJSON *oc_json_fmt_response(oc_json_rsp_t *response)
{
    cJSON *body = NULL;
    cJSON *root;
    cJSON *tmp;

    oc_json_t *kvlst;

    ///< create the root object
    root = cJSON_CreateObject();
    if (root == NULL) {
        goto EXIT_CJSON_ERR;
    }
    ///< create the msg type object and add it to the root
    tmp = cJSON_CreateString(OC_JSON_VAL_DeviceRsp);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_MsgType, tmp);
    ///< create the mid item and add it to the root
    tmp = cJSON_CreateNumber((double)response->mid);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_Mid, tmp);
    ///< create the err code item and add it to the root
    tmp = cJSON_CreateNumber((double)response->errcode);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_ErrCode, tmp);
    ///< create the has more item and add it to the root
    tmp = cJSON_CreateNumber((double)response->hasmore);
    if (tmp == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_HasMore, tmp);

    ///< create the body object and add it to root
    body = cJSON_CreateObject();
    if (body == NULL) {
        goto EXIT_CJSON_ERR;
    }
    cJSON_AddItemToObject(root, OC_JSON_KEY_Body, body);

    ///< create the body paras object and add it to body
    kvlst = response->bodylst;
    while (kvlst != NULL) {
        ///< fmt the value to a json object
        tmp = oc_json_fmt_value(kvlst);
        if (tmp == NULL) {
            goto EXIT_CJSON_ERR;
        }
        ///< add the object to the serice data
        cJSON_AddItemToObject(body, kvlst->key, tmp);

        kvlst = kvlst->next;
    }

    return root;

EXIT_CJSON_ERR:
    if (root) {
        cJSON_Delete(root);
    }

    return NULL;
}