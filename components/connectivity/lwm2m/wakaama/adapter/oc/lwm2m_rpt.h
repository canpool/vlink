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

#ifndef __LWM2M_RPT_H__
#define __LWM2M_RPT_H__

#include "liblwm2m.h"
#include "lwm2m_uri.h"
#include "vlist.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CONFIG_MAX_RPT_CNT
#define CONFIG_MAX_RPT_CNT  8
#endif

typedef enum {
    RPT_STAT_NOT_SENT = 0,
    RPT_STAT_SENT_WAIT_RESP,
    RPT_STAT_SENT_FAIL,
    RPT_STAT_SENT_TIMEOUT,
    RPT_STAT_SENT_SUCCESS,
    RPT_STAT_SENT_GET_RST,
    RPT_STAT_SEND_PENDING
} rpt_stat_e;

typedef struct rpt_list {
    vdlist_t            node;
    lwm2m_uri_t         uri;
    vdlist_t            queue;
    uint32_t            rpt_cnt;
    uint32_t            max_cnt;
} rpt_list_t;

typedef void (*rpt_notify_fn)(rpt_list_t *list, void *ctx);

struct rpt_data;
typedef struct rpt_data rpt_data_t;

typedef void (*rpt_ack_callback)(rpt_list_t *list, rpt_data_t *data, int status);

typedef struct rpt_data {
    int                 cookie;
    uint8_t            *buf;
    int                 len;
    rpt_ack_callback    callback;
} rpt_data_t;

typedef struct rpt_data_node {
    vdlist_t            node;
    rpt_list_t         *list;
    rpt_data_t          data;
} rpt_data_node_t;

int         lwm2m_rpt_init(void);
int         lwm2m_rpt_destroy(void);

rpt_list_t *lwm2m_rpt_add_uri(const lwm2m_uri_t *uri);
int         lwm2m_rpt_rm_uri(const lwm2m_uri_t *uri);

int         lwm2m_rpt_dequeue_data(rpt_list_t *list, rpt_data_t *data);
int         lwm2m_rpt_enqueue_data(const lwm2m_uri_t *uri, rpt_data_t *data);
int         lwm2m_rpt_clear_data(const lwm2m_uri_t *uri, int status);

int         lwm2m_rpt_set_max_cnt(const lwm2m_uri_t *uri, uint32_t max_cnt);

int         lwm2m_rpt_step(lwm2m_context_t *ctx, rpt_notify_fn notify);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LWM2M_RPT_H__ */