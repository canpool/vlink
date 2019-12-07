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

#include "lwm2m_port.h"
#include "object_comm.h"

#include <stdarg.h>

#include "vos.h"
#include "vmisc.h"

// devices
#define PRV_MANUFACTURER "Open Mobile Alliance"
#define PRV_MODEL_NUMBER "Lightweight M2M Client"
#define PRV_SERIAL_NUMBER "345000123"
#define PRV_FIRMWARE_VERSION "1.0"
#define PRV_POWER_SOURCE_1 1
#define PRV_POWER_SOURCE_2 5
#define PRV_POWER_VOLTAGE_1 3800
#define PRV_POWER_VOLTAGE_2 5000
#define PRV_POWER_CURRENT_1 125
#define PRV_POWER_CURRENT_2 900
#define PRV_BATTERY_LEVEL 100
#define PRV_MEMORY_FREE 15
#define PRV_ERROR_CODE 0
#define PRV_TIME_ZONE "Europe/Berlin"
#define PRV_BINDING_MODE "U"

#define VALUE_NETWORK_BEARER_GSM 0    // GSM see
#define VALUE_AVL_NETWORK_BEARER_1 0  // GSM
#define VALUE_AVL_NETWORK_BEARER_2 21 // WLAN
#define VALUE_AVL_NETWORK_BEARER_3 41 // Ethernet
#define VALUE_AVL_NETWORK_BEARER_4 42 // DSL
#define VALUE_AVL_NETWORK_BEARER_5 43 // PLC
#define VALUE_IP_ADDRESS_1 "192.168.178.101"
#define VALUE_IP_ADDRESS_2 "192.168.178.102"
#define VALUE_ROUTER_IP_ADDRESS_1 "192.168.178.001"
#define VALUE_ROUTER_IP_ADDRESS_2 "192.168.178.002"
#define VALUE_APN_1 "web.vodafone.de"
#define VALUE_APN_2 "cda.vodafone.de"
#define VALUE_CELL_ID 69696969
#define VALUE_RADIO_SIGNAL_STRENGTH 80 // dBm
#define VALUE_LINK_QUALITY 98
#define VALUE_LINK_UTILIZATION 666
#define VALUE_SMNC 33
#define VALUE_SMCC 44

#define PRV_POWER_VOLTAGE 3800
// #define PRV_BATTERY_LEVEL 90
// #define PRV_MEMORY_FREE 50
#define PRV_NETWORK_BEARER 5
#define PRV_SIGNAL_STRENGTH 90
#define PRV_CELL_ID 21103
#define PRV_LINK_QUALITY 98
#define PRV_LINK_UTRILIZATION 10
#define PRV_POWER_SOURCE 1
#define PRV_POWER_CURRENT 125
#define PRV_LATITUDE 27.986065f
#define PRV_LONGITUDE 86.922623f
#define PRV_ALTITUDE 8495.0000f
#define PRV_RADIUS 0.0f
#define PRV_SPEED 0.0f
#define PRV_TIME_CODE 1367491215

int lwm2m_cmd_get_manufacturer(char *manu, int len)
{
    (void)snprintf(manu, len, PRV_MANUFACTURER);
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_model_number(char *mode, int len)
{
    (void)snprintf(mode, len, PRV_MODEL_NUMBER);
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_serial_number(char *num, int len)
{
    (void)snprintf(num, len, PRV_SERIAL_NUMBER);
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_firmware_ver(char *version, int len)
{
    (void)snprintf(version, len, "example_ver001");
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_do_dev_reboot(void)
{
    (void)printf("device is rebooting......\r\n");
    // LOS_TaskDelay(1000);
    // osal_reboot();///< here may be we never comeback
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_do_factory_reset(void)
{
    (void)printf("\n\t FACTORY RESET\r\n\n");
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_power_source(int *arg)
{
    *arg = PRV_POWER_SOURCE;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_power_voltage(int *voltage)
{
    *voltage = PRV_POWER_VOLTAGE;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_power_current(int *arg)
{
    *arg = PRV_POWER_CURRENT;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_baterry_level(int *voltage)
{
    *voltage = PRV_BATTERY_LEVEL;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_memory_free(int *voltage)
{
    int tmp;
    //    (void)lwm2m_cmd_random(&tmp, sizeof(tmp));
    v_random(&tmp, sizeof(tmp));

    tmp %= 30;
    *voltage = PRV_MEMORY_FREE + tmp;
    return LWM2M_ERRNO_OK;
}

static int err_code = LWM2M_ERRNO_OK;

int lwm2m_cmd_get_dev_err(int *arg)
{
    *arg = err_code;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_do_reset_dev_err(void)
{
    err_code = LWM2M_ERRNO_OK;
    return LWM2M_ERRNO_OK;
}

static int64_t g_current_time = PRV_TIME_CODE;

int lwm2m_cmd_get_current_time(int64_t *arg)
{
    *arg = g_current_time + (int64_t)vos_sys_time() / 1000;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_set_current_time(const int64_t *arg)
{
    g_current_time = *arg - (int64_t)vos_sys_time() / 1000;
    return LWM2M_ERRNO_OK;
}

#define UTC_OFFSET_MAX_LEN 7
static char g_UTC_offset[UTC_OFFSET_MAX_LEN] = "+01:00";

int lwm2m_cmd_get_UTC_offset(char *offset, int len)
{
    if (len > strlen(g_UTC_offset) + 1) {
        (void)snprintf(offset, len, "%s", g_UTC_offset);
    }
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_set_UTC_offset(const char *offset, int len)
{
    (void)snprintf(g_UTC_offset, len + 1, "%s", offset);
    return LWM2M_ERRNO_OK;
}

#define TIMEZONE_MAXLEN 25
static char g_timezone[TIMEZONE_MAXLEN] = "Europe/Berlin";

int lwm2m_cmd_get_timezone(char *timezone, int len)
{
    if (len > strlen(g_timezone) + 1) {
        (void)snprintf(timezone, len, "%s", g_timezone);
    }
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_set_timezone(const char *timezone, int len)
{
    (void)snprintf(g_timezone, len + 1, "%s", timezone);
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_bind_mode(char *mode, int len)
{
    (void)printf("bind type is UQ......\r\n");
    (void)snprintf(mode, len, "UQ");
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_trig_firmware_update(void)
{
    (void)printf("firmware is updating......\r\n");
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_firmware_result(int *result)
{
    *result = 0;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_firmware_state(int *state)
{
    *state = 0;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_network_bearer(int *network_brearer)
{
    *network_brearer = PRV_NETWORK_BEARER;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_signal_strength(int *singal_strength)
{
    *singal_strength = PRV_SIGNAL_STRENGTH;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_cell_id(long *cell_id)
{
    *cell_id = PRV_CELL_ID;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_link_quality(int *quality)
{
    *quality = PRV_LINK_QUALITY;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_link_utilization(int *utilization)
{
    *utilization = PRV_LINK_UTRILIZATION;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_update_psk(char *psk_id, int len)
{
    // memcpy_s(g_psk_value,psk_id,len,16);
    (void)printf("update psk success\r\n");
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_latitude(float *latitude)
{
    *latitude = PRV_LATITUDE;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_longitude(float *longitude)
{
    *longitude = PRV_LONGITUDE;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_altitude(float *altitude)
{
    *altitude = PRV_ALTITUDE;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_radius(float *radius)
{
    *radius = PRV_RADIUS;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_speed(float *speed)
{
    *speed = PRV_SPEED;
    return LWM2M_ERRNO_OK;
}

int lwm2m_cmd_get_timestamp(uint64_t *timestamp)
{
    *timestamp = vos_sys_time() / 1000 + PRV_TIME_CODE;
    return LWM2M_ERRNO_OK;
}

//-----  3GPP TS 23.032 V11.0.0(2012-09) ---------
#define HORIZONTAL_VELOCITY 0
#define HORIZONTAL_VELOCITY_VERTICAL 1
#define HORIZONTAL_VELOCITY_WITH_UNCERTAINTY 2

#define VELOCITY_OCTETS 5

void location_get_velocity(lwm2m_velocity_s *velocity, uint16_t bearing, uint16_t horizontal_speed,
                           uint8_t speed_uncertainty)
{
    uint8_t tmp[VELOCITY_OCTETS];
    int copy_len;

    tmp[0] = HORIZONTAL_VELOCITY_WITH_UNCERTAINTY << 4;
    tmp[0] |= (bearing & 0x100) >> 8;
    tmp[1] = (bearing & 0xff);
    tmp[2] = horizontal_speed >> 8;
    tmp[3] = horizontal_speed & 0xff;
    tmp[4] = speed_uncertainty;

    copy_len = MAX_VELOCITY_LEN > sizeof(tmp) ? sizeof(tmp) : MAX_VELOCITY_LEN;
    memcpy(velocity->opaque, tmp, copy_len);
    velocity->length = copy_len;
}

int lwm2m_cmd_get_velocity(lwm2m_velocity_s *velocity)
{
    location_get_velocity(velocity, 0, 0, 255);
    return LWM2M_ERRNO_OK;
}

lwm2m_al_dealer_f s_default_dealer = NULL;

int lwm2m_cmd_register_dealer(lwm2m_al_dealer_f dealer)
{
    s_default_dealer = dealer;
    return 0;
}

static int lwm2m_cmd_default(char *msg, int len, va_list valist)
{
    int op = va_arg(valist, int);
    const char *uri = va_arg(valist, const char *);

    if (uri == NULL) {
        return -1;
    }

    if (s_default_dealer) {
        return s_default_dealer(op, uri, msg, len);
    } else {
        vlog_print("op: %d, uri: %s\n", op, uri);
    }
    return 0;
}

int lwm2m_cmd_ioctl(int cmd, char *arg, int len, ...)
{
    int result = LWM2M_ERRNO_OK;
    va_list valist;

    switch (cmd) {
    case LWM2M_CMD_GET_MANUFACTURER:
        result = lwm2m_cmd_get_manufacturer(arg, len);
        break;
    case LWM2M_CMD_GET_MODEL_NUMBER:
        result = lwm2m_cmd_get_model_number(arg, len);
        break;
    case LWM2M_CMD_GET_SERIAL_NUMBER:
        result = lwm2m_cmd_get_serial_number(arg, len);
        break;
    case LWM2M_CMD_GET_FIRMWARE_VER:
        result = lwm2m_cmd_get_firmware_ver(arg, len);
        break;
    case LWM2M_CMD_DO_DEV_REBOOT:
        result = lwm2m_cmd_do_dev_reboot();
        break;
    case LWM2M_CMD_DO_FACTORY_RESET:
        result = lwm2m_cmd_do_factory_reset();
        break;
    case LWM2M_CMD_GET_POWER_SOURCE:
        result = lwm2m_cmd_get_power_source((int *)arg);
        break;
    case LWM2M_CMD_GET_POWER_VOLTAGE:
        result = lwm2m_cmd_get_power_voltage((int *)arg);
        break;
    case LWM2M_CMD_GET_POWER_CURRENT:
        result = lwm2m_cmd_get_power_current((int *)arg);
        break;
    case LWM2M_CMD_GET_BATERRY_LEVEL:
        result = lwm2m_cmd_get_baterry_level((int *)arg);
        break;
    case LWM2M_CMD_GET_MEMORY_FREE:
        result = lwm2m_cmd_get_memory_free((int *)arg);
        break;
    case LWM2M_CMD_GET_DEV_ERR:
        result = lwm2m_cmd_get_dev_err((int *)arg);
        break;
    case LWM2M_CMD_DO_RESET_DEV_ERR:
        result = lwm2m_cmd_do_reset_dev_err();
        break;
    case LWM2M_CMD_GET_CURRENT_TIME:
        result = lwm2m_cmd_get_current_time((int64_t *)arg);
        break;
    case LWM2M_CMD_SET_CURRENT_TIME:
        result = lwm2m_cmd_set_current_time((const int64_t *)arg);
        break;
    case LWM2M_CMD_GET_UTC_OFFSET:
        result = lwm2m_cmd_get_UTC_offset(arg, len);
        break;
    case LWM2M_CMD_SET_UTC_OFFSET:
        result = lwm2m_cmd_set_UTC_offset(arg, len);
        break;
    case LWM2M_CMD_GET_TIMEZONE:
        result = lwm2m_cmd_get_timezone(arg, len);
        break;
    case LWM2M_CMD_SET_TIMEZONE:
        result = lwm2m_cmd_set_timezone(arg, len);
        break;
    case LWM2M_CMD_GET_BINDING_MODES:
        result = lwm2m_cmd_get_bind_mode(arg, len);
        break;
    case LWM2M_CMD_GET_FIRMWARE_STATE:
        result = lwm2m_cmd_get_firmware_state((int *)arg);
        break;
    case LWM2M_CMD_GET_NETWORK_BEARER:
        result = lwm2m_cmd_get_network_bearer((int *)arg);
        break;
    case LWM2M_CMD_GET_SIGNAL_STRENGTH:
        result = lwm2m_cmd_get_signal_strength((int *)arg);
        break;
    case LWM2M_CMD_GET_CELL_ID:
        result = lwm2m_cmd_get_cell_id((long *)arg);
        break;
    case LWM2M_CMD_GET_LINK_QUALITY:
        result = lwm2m_cmd_get_link_quality((int *)arg);
        break;
    case LWM2M_CMD_GET_LINK_UTILIZATION:
        result = lwm2m_cmd_get_link_utilization((int *)arg);
        break;
    case LWM2M_CMD_UPDATE_PSK:
        result = lwm2m_cmd_update_psk(arg, len);
        break;
    case LWM2M_CMD_GET_LATITUDE:
        result = lwm2m_cmd_get_latitude((float *)arg);
        break;
    case LWM2M_CMD_GET_LONGITUDE:
        result = lwm2m_cmd_get_longitude((float *)arg);
        break;
    case LWM2M_CMD_GET_ALTITUDE:
        result = lwm2m_cmd_get_altitude((float *)arg);
        break;
    case LWM2M_CMD_GET_RADIUS:
        result = lwm2m_cmd_get_radius((float *)arg);
        break;
    case LWM2M_CMD_GET_SPEED:
        result = lwm2m_cmd_get_speed((float *)arg);
        break;
    case LWM2M_CMD_GET_TIMESTAMP:
        result = lwm2m_cmd_get_timestamp((uint64_t *)arg);
        break;
    case LWM2M_CMD_GET_VELOCITY:
        result = lwm2m_cmd_get_velocity((lwm2m_velocity_s *)arg);
        break;

#ifdef CONFIG_FEATURE_FOTA
    case LWM2M_CMD_GET_OTA_OPT: {
        ota_opt_s *opt = (ota_opt_s *)arg;
        hal_get_ota_opt(opt);
        opt->key.rsa_N =
            "C94BECB7BCBFF459B9A71F12C3CC0603B11F0D3A366A226FD3E73D453F96EFBBCD4DFED6D9F77FD78C3AB1805E1BD3858131ACB530"
            "3F61AF524F43971B4D429CB847905E68935C1748D0096C1A09DD539CE74857F9FDF0B0EA61574C5D76BD9A67681AC6A9DB1BB22F17"
            "120B1DBF3E32633DCE34F5446F52DD7335671AC3A1F21DC557FA4CE9A4E0E3E99FED33A0BAA1C6F6EE53EDD742284D6582B51E4BF0"
            "19787B8C33C2F2A095BEED11D6FE68611BD00825AF97DB985C62C3AE0DC69BD7D0118E6D620B52AFD514AD5BFA8BAB998332213D7D"
            "BF5C98DC86CB8D4F98A416802B892B8D6BEE5D55B7E688334B281E4BEDDB11BD7B374355C5919BA5A9A1C91F";
        opt->key.rsa_E = "10001";
        result = LWM2M_ERRNO_OK;
        break;
    }
#endif

#if defined(WITH_AT_FRAMEWORK) && defined(USE_NB_NEUL95)
    case LWM2M_CMD_TRIGER_SERVER_INITIATED_BS:
        nb_reattach();
        result = LWM2M_ERRNO_OK;
        break;
#endif

    default:
        va_start(valist, len);
        result = lwm2m_cmd_default(arg, len, valist);
        va_end(valist);
        break;
    }
    return result;
}
