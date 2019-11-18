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

#ifndef __OC_JSON_H__
#define __OC_JSON_H__

#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* deviceReq data msg jason format example to server
the message can be decoded in json or binary.
{
        "msgType":      "deviceReq",
        "hasMore":      0,
        "data": [{
                        "serviceId":    "serviceIdValue",
                        "serviceData": {
                                "defineData": "defineValue"
                        },
                        "eventTime":    "20161219T114920Z"
                }]
}
*/
/*
cloudReq data msg jason format example from server
{
    "msgType":"cloudReq",
    "serviceId":"serviceIdValue",
    "paras":{
        "paraName":"paraValue"
    },
    "cmd":"cmdValue",
    "hasMore":0,
    "mid":0
}
*/
/*
deviceRsp data msg jason format example to server
{
    "msgType":  "deviceRsp",
    "mid":  0,
    "errcode":  0,
    "hasMore":  0,
    "body": {
        "bodyParaName": "bodyParaValue"
    }
}
*/

#define OC_JSON_KEY_MsgType         "msgType"
#define OC_JSON_KEY_HasMore         "hasMore"
#define OC_JSON_KEY_Data            "data"
#define OC_JSON_KEY_ServiceId       "serviceId"
#define OC_JSON_KEY_ServiceData     "serviceData"
#define OC_JSON_KEY_EventTime       "eventTime"
#define OC_JSON_KEY_Paras           "paras"
#define OC_JSON_KEY_Cmd             "cmd"
#define OC_JSON_KEY_Mid             "mid"
#define OC_JSON_KEY_ErrCode         "errcode"
#define OC_JSON_KEY_Body            "body"

#define OC_JSON_VAL_DeviceReq       "deviceReq"
#define OC_JSON_VAL_CloudReq        "cloudReq"
#define OC_JSON_VAL_DeviceRsp       "deviceRsp"

typedef enum {
    OC_JSON_TYPE_INT = 0,
    OC_JSON_TYPE_STRING,
    OC_JSON_TYPE_ARRAY
} oc_json_type_e;

typedef struct oc_json {
    struct oc_json  *next;
    oc_json_type_e   type;
    char            *key;
    char            *value;
    int              len; // value len
} oc_json_t;

typedef struct {
    char            *srvid;
    oc_json_t       *paralst;
    char            *eventtime;
    char             hasmore;
} oc_json_report_t;

typedef struct {
    int              mid;
    int              errcode;
    char             hasmore;
    oc_json_t       *bodylst;
} oc_json_rsp_t;

cJSON *oc_json_fmt_report(oc_json_report_t *report);
cJSON *oc_json_fmt_response(oc_json_rsp_t *response);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OC_JSON_H__ */