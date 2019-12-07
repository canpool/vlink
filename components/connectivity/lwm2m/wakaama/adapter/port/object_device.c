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
/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    domedambrosio - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Axel Lorente - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/

/*
 * This object is single instance only, and is mandatory to all LWM2M device as it describe the object such as its
 * manufacturer, model, etc...
 */

#include "object_comm.h"


#define PRV_OFFSET_MAXLEN   7 //+HH:MM\0 at max
#define PRV_TIMEZONE_MAXLEN 25
#define PRV_MAX_STRING_LEN  64

// Resource Id's:
#define RES_O_MANUFACTURER          0
#define RES_O_MODEL_NUMBER          1
#define RES_O_SERIAL_NUMBER         2
#define RES_O_FIRMWARE_VERSION      3
#define RES_M_REBOOT                4
#define RES_O_FACTORY_RESET         5
#define RES_O_AVL_POWER_SOURCES     6
#define RES_O_POWER_SOURCE_VOLTAGE  7
#define RES_O_POWER_SOURCE_CURRENT  8
#define RES_O_BATTERY_LEVEL         9
#define RES_O_MEMORY_FREE           10
#define RES_M_ERROR_CODE            11
#define RES_O_RESET_ERROR_CODE      12
#define RES_O_CURRENT_TIME          13
#define RES_O_UTC_OFFSET            14
#define RES_O_TIMEZONE              15
#define RES_M_BINDING_MODES         16
// since TS 20141126-C:
#define RES_O_DEVICE_TYPE           17
#define RES_O_HARDWARE_VERSION      18
#define RES_O_SOFTWARE_VERSION      19
#define RES_O_BATTERY_STATUS        20
#define RES_O_MEMORY_TOTAL          21

// basic check that the time offset value is at ISO 8601 format
// bug: +12:30 is considered a valid value by this function
static int prv_check_time_offset(char * buffer,
                                 int length)
{
    int min_index;

    if (length != 3 && length != 5 && length != 6) return 0;
    if (buffer[0] != '-' && buffer[0] != '+') return 0;
    switch (buffer[1])
    {
    case '0':
        if (buffer[2] < '0' || buffer[2] > '9') return 0;
        break;
    case '1':
        if (buffer[2] < '0' || buffer[2] > '2') return 0;
        break;
    default:
        return 0;
    }
    switch (length)
    {
    case 3:
        return 1;
    case 5:
        min_index = 3;
        break;
    case 6:
        if (buffer[3] != ':') return 0;
        min_index = 4;
        break;
    default:
        // never happen
        return 0;
    }
    if (buffer[min_index] < '0' || buffer[min_index] > '5') return 0;
    if (buffer[min_index+1] < '0' || buffer[min_index+1] > '9') return 0;

    return 1;
}

static uint8_t prv_set_value_power(lwm2m_data_t * dataP)
{
    lwm2m_data_t *subTlvP;
    int power[POWER_SOURCE_MAX_NUM] = {0};
    int power_num = 0;
    int result;
    int i;
    lwm2m_cmd_e cmd;

    switch (dataP->id)
    {
    case RES_O_AVL_POWER_SOURCES:
        cmd = LWM2M_CMD_GET_POWER_SOURCE;
        break;
    case RES_O_POWER_SOURCE_VOLTAGE:
        cmd = LWM2M_CMD_GET_POWER_VOLTAGE;
        break;
    case RES_O_POWER_SOURCE_CURRENT:
        cmd = LWM2M_CMD_GET_POWER_CURRENT;
        break;
    default:
        return COAP_400_BAD_REQUEST;
    }

    /* get power source num */
    result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_POWER_NUMBER, (char *)&power_num, sizeof(int));
    if (result != LWM2M_ERRNO_OK || power_num > POWER_SOURCE_MAX_NUM || power_num <= 0)
    {
        return COAP_400_BAD_REQUEST;
    }
    /* get source info */
    result = lwm2m_cmd_ioctl(cmd, (char *)power, sizeof(power));
    if (result != LWM2M_ERRNO_OK)
    {
        return COAP_400_BAD_REQUEST;
    }

    subTlvP = lwm2m_data_new(power_num);
    if (subTlvP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

    for (i = 0; i < power_num; ++i)
    {
        subTlvP[i].id = i;
        lwm2m_data_encode_int(power[i], subTlvP + i);
    }
    lwm2m_data_encode_instances(subTlvP, power_num, dataP);

    return COAP_205_CONTENT;
}

static uint8_t prv_set_value_error_code(lwm2m_data_t * dataP)
{
    lwm2m_data_t * subTlvP;
    int error;
    int result;

    result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_DEV_ERR, (char *)&error, sizeof(int));
    if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;

    subTlvP = lwm2m_data_new(1);
    if (subTlvP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

    subTlvP[0].id = 0;
    lwm2m_data_encode_int(error, subTlvP);

    lwm2m_data_encode_instances(subTlvP, 1, dataP);

    return COAP_205_CONTENT;
}

static uint8_t prv_set_value(lwm2m_data_t * dataP)
{
    char str[PRV_MAX_STRING_LEN + 1] = {0};
    char time_offset[PRV_OFFSET_MAXLEN] = {0};
    char time_zone[PRV_TIMEZONE_MAXLEN] = {0};
    int64_t current_time;
    int battery_level;
    int free_memory;
    int result;

    // a simple switch structure is used to respond at the specified resource asked
    switch (dataP->id)
    {
    case RES_O_MANUFACTURER:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_MANUFACTURER, str, sizeof(str));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(str, dataP);
        return COAP_205_CONTENT;

    case RES_O_MODEL_NUMBER:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_MODEL_NUMBER, str, sizeof(str));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(str, dataP);
        return COAP_205_CONTENT;

    case RES_O_SERIAL_NUMBER:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_SERIAL_NUMBER, str, sizeof(str));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(str, dataP);
        return COAP_205_CONTENT;

    case RES_O_FIRMWARE_VERSION:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_FIRMWARE_VER, str, sizeof(str));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(str, dataP);
        return COAP_205_CONTENT;

    case RES_M_REBOOT:
        return COAP_405_METHOD_NOT_ALLOWED;

    case RES_O_FACTORY_RESET:
        return COAP_405_METHOD_NOT_ALLOWED;

    case RES_O_AVL_POWER_SOURCES:
    case RES_O_POWER_SOURCE_VOLTAGE:
    case RES_O_POWER_SOURCE_CURRENT:
        return prv_set_value_power(dataP);

    case RES_O_BATTERY_LEVEL:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_BATERRY_LEVEL, (char *)&battery_level, sizeof(int));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_int(battery_level, dataP);
        return COAP_205_CONTENT;

    case RES_O_MEMORY_FREE:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_MEMORY_FREE, (char *)&free_memory, sizeof(int));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_int(free_memory, dataP);
        return COAP_205_CONTENT;

    case RES_M_ERROR_CODE:
        return prv_set_value_error_code(dataP);

    case RES_O_RESET_ERROR_CODE:
        return COAP_405_METHOD_NOT_ALLOWED;

    case RES_O_CURRENT_TIME:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_CURRENT_TIME, (char *)&current_time, sizeof(int64_t));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_int(current_time, dataP);
        return COAP_205_CONTENT;

    case RES_O_UTC_OFFSET:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_UTC_OFFSET, time_offset, sizeof(time_offset));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(time_offset, dataP);
        return COAP_205_CONTENT;

    case RES_O_TIMEZONE:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_TIMEZONE, time_zone, sizeof(time_zone));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(time_zone, dataP);
        return COAP_205_CONTENT;

    case RES_M_BINDING_MODES:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_GET_BINDING_MODES, str, sizeof(str));
        if (result != LWM2M_ERRNO_OK) return COAP_400_BAD_REQUEST;
        lwm2m_data_encode_string(str, dataP);
        return COAP_205_CONTENT;

    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_device_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_O_MANUFACTURER,
                RES_O_MODEL_NUMBER,
                RES_O_SERIAL_NUMBER,
                RES_O_FIRMWARE_VERSION,
                //E: RES_M_REBOOT,
                //E: RES_O_FACTORY_RESET,
                RES_O_AVL_POWER_SOURCES,
                RES_O_POWER_SOURCE_VOLTAGE,
                RES_O_POWER_SOURCE_CURRENT,
                RES_O_BATTERY_LEVEL,
                RES_O_MEMORY_FREE,
                RES_M_ERROR_CODE,
                //E: RES_O_RESET_ERROR_CODE,
                RES_O_CURRENT_TIME,
                RES_O_UTC_OFFSET,
                RES_O_TIMEZONE,
                RES_M_BINDING_MODES
        };
        int nbRes = sizeof(resList)/sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        result = prv_set_value((*dataArrayP) + i);
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

static uint8_t prv_device_discover(uint16_t instanceId,
                                   int * numDataP,
                                   lwm2m_data_t ** dataArrayP,
                                   lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    result = COAP_205_CONTENT;

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
            RES_O_MANUFACTURER,
            RES_O_MODEL_NUMBER,
            RES_O_SERIAL_NUMBER,
            RES_O_FIRMWARE_VERSION,
            RES_M_REBOOT,
            RES_O_FACTORY_RESET,
            RES_O_AVL_POWER_SOURCES,
            RES_O_POWER_SOURCE_VOLTAGE,
            RES_O_POWER_SOURCE_CURRENT,
            RES_O_BATTERY_LEVEL,
            RES_O_MEMORY_FREE,
            RES_M_ERROR_CODE,
            RES_O_RESET_ERROR_CODE,
            RES_O_CURRENT_TIME,
            RES_O_UTC_OFFSET,
            RES_O_TIMEZONE,
            RES_M_BINDING_MODES
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }
    else
    {
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
        {
            switch ((*dataArrayP)[i].id)
            {
            case RES_O_MANUFACTURER:
            case RES_O_MODEL_NUMBER:
            case RES_O_SERIAL_NUMBER:
            case RES_O_FIRMWARE_VERSION:
            case RES_M_REBOOT:
            case RES_O_FACTORY_RESET:
            case RES_O_AVL_POWER_SOURCES:
            case RES_O_POWER_SOURCE_VOLTAGE:
            case RES_O_POWER_SOURCE_CURRENT:
            case RES_O_BATTERY_LEVEL:
            case RES_O_MEMORY_FREE:
            case RES_M_ERROR_CODE:
            case RES_O_RESET_ERROR_CODE:
            case RES_O_CURRENT_TIME:
            case RES_O_UTC_OFFSET:
            case RES_O_TIMEZONE:
            case RES_M_BINDING_MODES:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

static uint8_t prv_device_write(uint16_t instanceId,
                                int numData,
                                lwm2m_data_t * dataArray,
                                lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
    int64_t current_time;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;

    do
    {
        switch (dataArray[i].id)
        {
        case RES_O_CURRENT_TIME:
            if (1 == lwm2m_data_decode_int(dataArray + i, &current_time))
            {
                result = lwm2m_cmd_ioctl(LWM2M_CMD_SET_CURRENT_TIME, (char *)&current_time, sizeof(int64_t));
                if (result == LWM2M_ERRNO_OK)
                {
                    return COAP_204_CHANGED;
                }
                else
                {
                    return COAP_400_BAD_REQUEST;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;

        case RES_O_UTC_OFFSET:
            if (1 == prv_check_time_offset((char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length))
            {
                result = lwm2m_cmd_ioctl(LWM2M_CMD_SET_UTC_OFFSET, (char *)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                if (result == LWM2M_ERRNO_OK)
                {
                    return COAP_204_CHANGED;
                }
                else
                {
                    return COAP_400_BAD_REQUEST;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;

        case RES_O_TIMEZONE:
            result = lwm2m_cmd_ioctl(LWM2M_CMD_SET_TIMEZONE, (char *)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
            if (result == LWM2M_ERRNO_OK)
            {
                return COAP_204_CHANGED;
            }
            else
            {
                return COAP_400_BAD_REQUEST;
            }

        default:
            result = COAP_405_METHOD_NOT_ALLOWED;
        }

        i++;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}

static uint8_t prv_device_execute(uint16_t instanceId,
                                  uint16_t resourceId,
                                  uint8_t * buffer,
                                  int length,
                                  lwm2m_object_t * objectP)
{
    int result;
    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    if (length != 0) return COAP_400_BAD_REQUEST;

    switch (resourceId)
    {
    case RES_M_REBOOT:
        // g_reboot = 1;
        return COAP_204_CHANGED;
    case RES_O_FACTORY_RESET:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_DO_FACTORY_RESET, NULL, 0);
        if (result == LWM2M_ERRNO_OK)
        {
            return COAP_204_CHANGED;
        }
        else
        {
            return COAP_503_SERVICE_UNAVAILABLE;
        }
    case RES_O_RESET_ERROR_CODE:
        result = lwm2m_cmd_ioctl(LWM2M_CMD_DO_RESET_DEV_ERR, NULL, 0);
        if (result == LWM2M_ERRNO_OK)
        {
            return COAP_204_CHANGED;
        }
        else
        {
            return COAP_503_SERVICE_UNAVAILABLE;
        }
    default:
        return COAP_405_METHOD_NOT_ALLOWED;
    }
}

void display_device_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Device object:\r\n", object->objID);
#endif
}

lwm2m_object_t * get_object_device(void)
{
    /*
     * The get_object_device function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * deviceObj;

    deviceObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != deviceObj)
    {
        memset(deviceObj, 0, sizeof(lwm2m_object_t));

        /*
         * It assigns his unique ID
         * The 3 is the standard ID for the mandatory object "Object device".
         */
        deviceObj->objID = LWM2M_DEVICE_OBJECT_ID;

        /*
         * and its unique instance
         *
         */
        deviceObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != deviceObj->instanceList)
        {
            memset(deviceObj->instanceList, 0, sizeof(lwm2m_list_t));
        }
        else
        {
            lwm2m_free(deviceObj);
            return NULL;
        }

        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        deviceObj->readFunc     = prv_device_read;
        deviceObj->discoverFunc = prv_device_discover;
        deviceObj->writeFunc    = prv_device_write;
        deviceObj->executeFunc  = prv_device_execute;
    }

    return deviceObj;
}

void free_object_device(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList)
    {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }

    lwm2m_free(objectP);
}

static lwm2m_object_t * lwm2m_get_device_object(void)
{
    lwm2m_object_t * deviceObj;

    deviceObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    if (deviceObj == NULL) {
        return NULL;
    }

    memset(deviceObj, 0, sizeof(lwm2m_object_t));
    deviceObj->objID = LWM2M_DEVICE_OBJECT_ID;

    deviceObj->readFunc     = prv_device_read;
    deviceObj->discoverFunc = prv_device_discover;
    deviceObj->writeFunc    = prv_device_write;
    deviceObj->executeFunc  = prv_device_execute;

    return deviceObj;
}

static int lwm2m_add_device_instance(lwm2m_object_t *obj, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    int ret = LWM2M_ERRNO_OK;

    lwm2m_list_t * deviceInstance;

    deviceInstance = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
    if (deviceInstance == NULL) {
        return LWM2M_ERRNO_NOMEM;
    }
    memset(deviceInstance, 0, sizeof(lwm2m_list_t));

    deviceInstance->id = uri->obj_id;

    obj->instanceList = LWM2M_LIST_ADD(obj->instanceList, deviceInstance);

    return ret;
}

int lwm2m_add_device_object(lwm2m_context_t *ctx, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    int ret = LWM2M_ERRNO_OK;
    bool is_new = false;

    lwm2m_object_t * obj = (lwm2m_object_t *)LWM2M_LIST_FIND(ctx->objectList, uri->obj_id);
    if (obj == NULL) {
        obj = lwm2m_get_device_object();
        if (obj == NULL) {
            return LWM2M_ERRNO_NOMEM;
        }
        is_new = true;
    }

    lwm2m_list_t *inst = LWM2M_LIST_FIND(obj->instanceList, uri->inst_id);
    if (inst == NULL) {
        // single instance
        if (obj->instanceList != NULL || uri->inst_id != 0) {
            return LWM2M_ERRNO_PERM;
        }
        ret = lwm2m_add_device_instance(obj, uri, obj_data);
        if (ret != LWM2M_ERRNO_OK) {
            if (is_new) {
                lwm2m_free(obj);
            }
            return ret;
        }
    }
    // ignore resources

    if (is_new) {
        lwm2m_add_object(ctx, obj);
    }

    return ret;
}
