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

#ifndef __AGENT_COAP_H__
#define __AGENT_COAP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AGENT_OK               0
#define AGENT_ERR             -1

#define AGENT_MAX_SN_LEN       16
#define AGENT_MAX_ADDR_LEN     16

#define AGENT_BIND_TIMEOUT     4000 /* ms */

/*
    oclink internel running state
    INIT--->BINDING
    BINDING--->WAIT_BINDOK
    WAIT_BINDOK ----> IDLE
    IDLE ---> IDLE
    IDLE ---> ERR
    ERR ---> BINDING
*/
#define AGENT_STAT_INIT        0
#define AGENT_STAT_BINDING     1
#define AGENT_STAT_WAIT_BINDOK 2
#define AGENT_STAT_IDLE        3
#define AGENT_STAT_ERR         4
#define AGENT_STAT_MAX         5

#define AGENT_ERR_CODE_NODATA      1 /* can't get udp data over 2 scconds. */
#define AGENT_ERR_CODE_NETWORK     2 /* may be server address err casuse this err. */
#define AGENT_ERR_CODE_BINDTIMEOUT 3 /* bind process timeout(over 5 seconds), need redo bind process. */
#define AGENT_ERR_CODE_DEVNOTEXSIT 4 /* device(identify by the sn) not registed to the platform, or your sn. */

int agent_errno_get(void);

int oc_coap_install_agent(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AGENT_COAP_H__ */