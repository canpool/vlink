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

#include "lwm2m_rpt.h"

#include <string.h>

#include "vos.h"


static vdlist_t     s_rpt_table;
static vmutex_t     s_mutex = V_MUTEX_INVALID;


static rpt_list_t *lwm2m_rpt_find(const lwm2m_uri_t *uri)
{
    vdlist_t *item, *next;

    V_DLIST_FOR_EACH_SAFE(item, next, &s_rpt_table) {
        rpt_list_t *rpt = (rpt_list_t *)item;
        if (lwm2m_uri_equal(&rpt->uri, uri)) {
            return rpt;
        }
    }

    return NULL;
}

static void lwm2m_rpt_free(vdlist_t *list, void (*free_fn)(vdlist_t *node, uintptr_t arg), uintptr_t arg)
{
    vdlist_t *item, *next;

    V_DLIST_FOR_EACH_SAFE(item, next, list) {
        vdlist_del(item);
        if (free_fn) {
            free_fn(item, arg);
        }
        lwm2m_free(item);
    }
}

static void lwm2m_rpt_visit(vdlist_t *list, rpt_notify_fn notify_fn, void *arg)
{
    vdlist_t *item, *next;

    V_DLIST_FOR_EACH_SAFE(item, next, list) {
        if (notify_fn) {
            notify_fn((rpt_list_t *)item, arg);
        }
    }
}

static void lwm2m_rpt_free_data(vdlist_t *node, uintptr_t arg)
{
    rpt_data_node_t *rptnode = (rpt_data_node_t *)node;

    if (rptnode->data.callback) {
        rptnode->data.callback(rptnode->list, &rptnode->data, (int)arg);
    }
    if (rptnode->data.buf) {
        lwm2m_free(rptnode->data.buf);
        rptnode->data.buf = NULL;
    }
}

static void lwm2m_rpt_clear(vdlist_t *node, uintptr_t arg)
{
    rpt_list_t *list = (rpt_list_t *)node;
    lwm2m_rpt_free(&list->queue, lwm2m_rpt_free_data, arg);
}

int lwm2m_rpt_init(void)
{
    vdlist_init(&s_rpt_table);

    if (vos_mutex_init(&s_mutex) != 0) {
        return -1;
    }
    return 0;
}

int lwm2m_rpt_destroy(void)
{
    lwm2m_rpt_free(&s_rpt_table, lwm2m_rpt_clear, (uintptr_t)RPT_STAT_NOT_SENT);
    return vos_mutex_destroy(&s_mutex);
}

rpt_list_t *lwm2m_rpt_add_uri(const lwm2m_uri_t *uri)
{
    rpt_list_t *rptlst = NULL;

    if (uri == NULL) {
        return rptlst;
    }

    vos_mutex_lock(s_mutex);

    do {
        rptlst = lwm2m_rpt_find(uri);
        if (rptlst != NULL) {
            vlog_error("uri exist," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }

        rptlst = (rpt_list_t *)lwm2m_malloc(sizeof(rpt_list_t));
        if (rptlst == NULL) {
            vlog_error("lwm2m_malloc fail," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }

        memset(rptlst, 0, sizeof(rpt_list_t));
        vdlist_init(&rptlst->queue);
        rptlst->uri = *uri;
        vdlist_add_tail(&s_rpt_table, &rptlst->node);
    } while (0);

    vos_mutex_unlock(s_mutex);

    return rptlst;
}

int lwm2m_rpt_rm_uri(const lwm2m_uri_t *uri)
{
    int ret = -1;

    if (uri == NULL) {
        return ret;
    }

    vos_mutex_lock(s_mutex);

    do {
        rpt_list_t *rptlst = lwm2m_rpt_find(uri);
        if (rptlst == NULL) {
            vlog_error("uri not exist," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }

        vdlist_del(&rptlst->node);
        lwm2m_rpt_free(&rptlst->queue, lwm2m_rpt_free_data, (uintptr_t)RPT_STAT_NOT_SENT);
        lwm2m_free(rptlst);

        ret = 0;
    } while (0);

    vos_mutex_unlock(s_mutex);

    return ret;
}

int lwm2m_rpt_dequeue_data(rpt_list_t *list, rpt_data_t *data)
{
    int ret = -1;

    if (list == NULL || data == NULL) {
        return ret;
    }

    vos_mutex_lock(s_mutex);

    do {
        if (vdlist_empty(&list->queue)) {
            break;
        }
        rpt_data_node_t *rptnode = (rpt_data_node_t *)list->queue.next;
        vdlist_del(&rptnode->node);
        if (list->rpt_cnt > 0) {
            list->rpt_cnt--;
        }
        *data = rptnode->data;
        lwm2m_free(rptnode);

        ret = 0;
    } while (0);

    vos_mutex_unlock(s_mutex);

    return ret;
}

int lwm2m_rpt_enqueue_data(const lwm2m_uri_t *uri, rpt_data_t *data)
{
    int ret = -1;

    if (uri == NULL || data == NULL) {
        return ret;
    }

    vos_mutex_lock(s_mutex);

    do {
        rpt_list_t *rptlst = lwm2m_rpt_find(uri);
        if (rptlst == NULL) {
            vlog_print("uri not exist," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }
        if (rptlst->rpt_cnt >= rptlst->max_cnt) {
            vlog_print("rpt cnt %u, max cnt %u," URI_FORMAT,
                rptlst->rpt_cnt, rptlst->max_cnt, URI_LOG_PARAM(uri));
            break;
        }
        rpt_data_node_t *rptnode = (rpt_data_node_t *)lwm2m_malloc(sizeof(rpt_data_node_t));
        if (rptnode == NULL) {
            vlog_error("lwm2m_malloc fail," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }

        memset(rptnode, 0, sizeof(rpt_data_node_t));
        rptnode->data = *data;
        rptnode->list = rptlst;
        vdlist_add_tail(&rptlst->queue, &rptnode->node);
        rptlst->rpt_cnt++;

        ret = 0;
    } while (0);

    vos_mutex_unlock(s_mutex);

    return ret;
}

int lwm2m_rpt_clear_data(const lwm2m_uri_t *uri, int status)
{
    int ret = -1;

    if (uri == NULL) {
        return ret;
    }

    vos_mutex_lock(s_mutex);

    do {
        rpt_list_t *rptlst = lwm2m_rpt_find(uri);
        if (rptlst == NULL) {
            vlog_error("uri not exist," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }
        lwm2m_rpt_free(&rptlst->queue, lwm2m_rpt_free_data, (uintptr_t)status);
        rptlst->rpt_cnt = 0;

        ret = 0;
    } while (0);

    vos_mutex_unlock(s_mutex);

    return ret;
}

int lwm2m_rpt_set_max_cnt(const lwm2m_uri_t *uri, uint32_t max_cnt)
{
    int ret = -1;

    if (uri == NULL) {
        return ret;
    }

    vos_mutex_lock(s_mutex);

    do {
        rpt_list_t *rptlst = lwm2m_rpt_find(uri);
        if (rptlst == NULL) {
            vlog_error("uri not exist," URI_FORMAT, URI_LOG_PARAM(uri));
            break;
        }
        rptlst->max_cnt = max_cnt;
        ret = 0;
    } while (0);

    vos_mutex_unlock(s_mutex);

    return ret;
}

int lwm2m_rpt_step(lwm2m_context_t *ctx, rpt_notify_fn notify)
{
    if (ctx == NULL) {
        return -1;
    }
    vos_mutex_lock(s_mutex);
    lwm2m_rpt_visit(&s_rpt_table, notify, ctx);
    vos_mutex_unlock(s_mutex);

    return 0;
}