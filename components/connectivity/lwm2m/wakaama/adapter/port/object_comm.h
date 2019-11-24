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

#ifndef __OBJECT_COMM_H__
#define __OBJECT_COMM_H__

#include "internals.h"
#include "connection.h"
#include "lwm2m_al.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /* __cplusplus */

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

typedef enum {
    LWM2M_ERRNO_OK = 0,
    LWM2M_ERRNO_ERR = -1,
    LWM2M_ERRNO_INVAL = -2,
    LWM2M_ERRNO_NOMEM = -3,
    LWM2M_ERRNO_OVERFLOW = -4,
    LWM2M_ERRNO_NORES = -5,
} lwm2m_errno_e;

/**
 * Resoure operations (LwM2M cmd)
 *  Read    - GET
 *  Write   - SET
 *  Excute  - DO
 *
 * R.S.O - Read.Single.Optional
 * R.S.M - Read.Single.Mandatory
 * E.S.M - Excute.Single.Mandatory
 * R.M.M - Read.Multiple.Mandatory
 */
typedef enum
{
    /* LwM2M object: Device */
    LWM2M_CMD_GET_MANUFACTURER,     /* R.S.O string */
    LWM2M_CMD_GET_MODEL_NUMBER,     /* R.S.O string */
    LWM2M_CMD_GET_SERIAL_NUMBER,    /* R.S.O string */
    LWM2M_CMD_GET_FIRMWARE_VER,     /* R.S.O string */
    LWM2M_CMD_DO_DEV_REBOOT,        /* E.S.M */
    LWM2M_CMD_DO_FACTORY_RESET,     /* E.S.O */
    LWM2M_CMD_GET_POWER_NUMBER,     /* Get the number of available power soucrces */
    LWM2M_CMD_GET_POWER_SOURCE,     /* R.M.O Integer, 0..7 */
    LWM2M_CMD_GET_POWER_VOLTAGE,    /* R.M.O Integer, The unit is mV */
    LWM2M_CMD_GET_POWER_CURRENT,    /* R.M.O Integer, The unit is mA */
    LWM2M_CMD_GET_BATERRY_LEVEL,    /* R.S.O Integer, 0..100%, only valid for Power Source 1 (Internal Battery) */
    LWM2M_CMD_GET_MEMORY_FREE,      /* R.S.O Integer, Estimated current available amount of storage space (kilobytes) */
    LWM2M_CMD_GET_DEV_ERR,          /* R.M.M Integer, 0..8, Error Code */
    LWM2M_CMD_DO_RESET_DEV_ERR,     /* E.S.O */
    LWM2M_CMD_GET_CURRENT_TIME,     /* RW.S.O Time */
    LWM2M_CMD_SET_CURRENT_TIME,
    LWM2M_CMD_GET_UTC_OFFSET,       /* RW.S.O string */
    LWM2M_CMD_SET_UTC_OFFSET,
    LWM2M_CMD_GET_TIMEZONE,         /* RW.S.O string */
    LWM2M_CMD_SET_TIMEZONE,
    LWM2M_CMD_GET_BINDING_MODES,    /* R.S.M string */
                                    /* remains: Device Type, Hardware Version, Software Version,
                                                Battery Status, Memory Total, ExtDevInfo
                                    */
    LWM2M_CMD_GET_FIRMWARE_STATE,
    LWM2M_CMD_GET_NETWORK_BEARER,
    LWM2M_CMD_GET_SIGNAL_STRENGTH,
    LWM2M_CMD_GET_CELL_ID,
    LWM2M_CMD_GET_LINK_QUALITY,
    LWM2M_CMD_GET_LINK_UTILIZATION,
    LWM2M_CMD_WRITE_APP_DATA,
    LWM2M_CMD_EXECUTE_APP_DATA,
    LWM2M_CMD_UPDATE_PSK,
    LWM2M_CMD_GET_LATITUDE,
    LWM2M_CMD_GET_LONGITUDE,
    LWM2M_CMD_GET_ALTITUDE,
    LWM2M_CMD_GET_RADIUS,
    LWM2M_CMD_GET_SPEED,
    LWM2M_CMD_GET_TIMESTAMP,
    LWM2M_CMD_GET_VELOCITY,
    LWM2M_CMD_GET_OTA_OPT,
    LWM2M_CMD_TRIGER_SERVER_INITIATED_BS
} lwm2m_cmd_e;

int lwm2m_cmd_ioctl(int cmd, char *arg, int len);

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    lwm2m_context_t * lwm2mH;
    connection_t * connList;
    void * observe_mutex;
} client_data_t;

#define MAX_VELOCITY_LEN    16

typedef struct {
    uint8_t opaque[MAX_VELOCITY_LEN];
    int     length;
} lwm2m_velocity_s;

/*
 * object_device.c
 */
typedef enum power_source
{
    POWER_SOURCE_DC = 0,        /* DC power */
    POWER_SOURCE_INTER = 1,     /* Internal Battery */
    POWER_SOURCE_EXTER = 2,     /* External Battery */
    POWER_SOURCE_FUEL = 3,      /* Fuel Cell */
    POWER_SOURCE_ETH = 4,       /* Power over Ethernet */
    POWER_SOURCE_USB = 5,       /* USB */
    POWER_SOURCE_AC = 6,        /* AC(Mains) power */
    POWER_SOURCE_SOLAR = 7,     /* Solar */
    POWER_SOURCE_MAX_NUM
} power_source_e;

typedef enum dev_err
{
    DEV_ERR_NO_ERR = 0,                     /* No error */
    DEV_ERR_BAT_POWER_LOW = 1,              /* Low battery power */
    DEV_ERR_EXT_POWER_OFF = 2,              /* External power supply off */
    DEV_ERR_GPS_FAILURE = 3,                /* GPS module failure */
    DEV_ERR_SIGNAL_STRENGTH_LOW = 4,        /* Low received signal strength */
    DEV_ERR_OUT_OF_MEMORY = 5,              /* Out of memory */
    DEV_ERR_SMS_FAILURE = 6,                /* SMS failure */
    DEV_ERR_IP_CONN_FAILURE = 7,            /* IP connectivity failure */
    DEV_ERR_PERIPHERAL_MALFUNCTION = 8,     /* Peripheral malfunction */
    DEV_ERR_MAX_NUM
} dev_err_e;

lwm2m_object_t * get_object_device(void);
void free_object_device(lwm2m_object_t * objectP);
uint8_t device_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
void display_device_object(lwm2m_object_t * objectP);
/*
 * object_firmware.c
 */
lwm2m_object_t * get_object_firmware(void);
void free_object_firmware(lwm2m_object_t * objectP);
void display_firmware_object(lwm2m_object_t * objectP);
/*
 * object_location.c
 */
lwm2m_object_t * get_object_location(void);
void free_object_location(lwm2m_object_t * object);
void display_location_object(lwm2m_object_t * objectP);
/*
 * object_test.c
 */
#define TEST_OBJECT_ID 31024
lwm2m_object_t * get_test_object(void);
void free_test_object(lwm2m_object_t * object);
void display_test_object(lwm2m_object_t * objectP);
/*
 * object_server.c
 */
lwm2m_object_t * get_server_object(int serverId, const char* binding, int lifetime, bool storing);
void clean_server_object(lwm2m_object_t * object);
void free_server_object(lwm2m_object_t * object);
void display_server_object(lwm2m_object_t * objectP);
void copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_connectivity_moni.c
 */
lwm2m_object_t * get_object_conn_m(void);
void free_object_conn_m(lwm2m_object_t * objectP);
uint8_t connectivity_moni_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);

/*
 * object_connectivity_stat.c
 */
extern lwm2m_object_t * get_object_conn_s(void);
void free_object_conn_s(lwm2m_object_t * objectP);
extern void conn_s_updateTxStatistic(lwm2m_object_t * objectP, uint16_t txDataByte, bool smsBased);
extern void conn_s_updateRxStatistic(lwm2m_object_t * objectP, uint16_t rxDataByte, bool smsBased);

/*
 * object_access_control.c
 */
lwm2m_object_t* acc_ctrl_create_object(void);
void acl_ctrl_free_object(lwm2m_object_t * objectP);
bool  acc_ctrl_obj_add_inst (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t acObjectId, uint16_t acObjInstId, uint16_t acOwner);
bool  acc_ctrl_oi_add_ac_val(lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t aclResId, uint16_t acValue);

/*
 * object_security.c
 */
typedef struct _security_instance_
{
    struct _security_instance_ * next;        // matches lwm2m_list_t::next
    uint16_t                     instanceId;  // matches lwm2m_list_t::id
    char *                       uri;
    bool                         isBootstrap;
    uint8_t                      securityMode;
    char *                       publicIdentity;
    uint16_t                     publicIdLen;
    char *                       serverPublicKey;
    uint16_t                     serverPublicKeyLen;
    char *                       secretKey;
    uint16_t                     secretKeyLen;
    uint8_t                      smsSecurityMode;
    char *                       smsParams; // SMS binding key parameters
    uint16_t                     smsParamsLen;
    char *                       smsSecret; // SMS binding secret key
    uint16_t                     smsSecretLen;
    uint16_t                     shortID;
    uint32_t                     clientHoldOffTime;
    uint32_t                     bootstrapServerAccountTimeout;
} security_instance_t;

lwm2m_object_t * get_security_object(int serverId, lwm2m_al_config_t *config);
void clean_security_object(lwm2m_object_t * objectP);
void free_security_object(lwm2m_object_t * objectP);
char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
void display_security_object(lwm2m_object_t * objectP);
void copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_binary_app_data.c
 */
#define BINARY_APP_DATA_OBJECT_ID   19
#define BINARY_APP_DATA_RES_ID      0
lwm2m_object_t * get_binary_app_data_object(lwm2m_al_config_t *config);
void free_binary_app_data_object(lwm2m_object_t * object);
void display_binary_app_data_object(lwm2m_object_t * objectP);
void set_binary_app_data_object_rpt_max_cnt(uint32_t max_rpt_cnt);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* __cplusplus */


#endif /* __OBJECT_COMM_H__ */
