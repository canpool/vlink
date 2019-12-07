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
#include "commandline.h"

#include "lwm2m_al.h"
#include "lwm2m_rpt.h"

#include "vos.h"

typedef struct {
    lwm2m_context_t *lwm2m_context;
    client_data_t client_data;
    uint8_t *recv_buf;
    vtask_t task;
    int quit_flag;
} lwm2m_cb_t;


static int lwm2m_init_client_data(lwm2m_cb_t *cb)
{
    client_data_t *pdata = &cb->client_data;

    pdata->observe_mutex = (void *)V_MUTEX_INVALID;

    if (vos_mutex_init((vmutex_t *)&pdata->observe_mutex) != 0) {
        return LWM2M_ERRNO_NORES;
    }

    pdata->lwm2mH = cb->lwm2m_context;

    return LWM2M_ERRNO_OK;
}

static int lwm2m_destroy_client_data(lwm2m_cb_t *cb)
{
    client_data_t *pdata = &cb->client_data;

    return vos_mutex_destroy((vmutex_t *)&pdata->observe_mutex);
}

static int lwm2m_poll(lwm2m_cb_t *cb, uint32_t timeout)
{
    client_data_t *dataP;
    int numBytes;
    connection_t *connP;
    lwm2m_context_t *contextP = cb->lwm2m_context;
    uint8_t *recv_buffer = cb->recv_buf;

    dataP = (client_data_t *)(contextP->userData);
    connP = dataP->connList;

    while (connP != NULL)
    {
        numBytes = lwm2m_buffer_recv(connP, recv_buffer, MAX_PACKET_SIZE, timeout);
        if (numBytes > 0) {
            output_buffer(stderr, recv_buffer, numBytes, 0);
            lwm2m_handle_packet(contextP, recv_buffer, numBytes, connP);
        }
        connP = connP->next;
    }

    return LWM2M_ERRNO_OK;
}

static void lwm2m_rpt_ack_callback(rpt_list_t *list, rpt_data_t *data, int status)
{
    vlog_print("cookie: %d\n", data->cookie);
}

static void lwm2m_rpt_notify(rpt_list_t *list, void *ctx)
{
    if (list == NULL || ctx == NULL) {
        return;
    }
    if (!vdlist_empty(&list->queue)) {
        lwm2m_resource_value_changed((lwm2m_context_t *)ctx, &list->uri);
    }
}

static void __lwm2m_disconnect(lwm2m_cb_t *cb)
{
    cb->quit_flag = 0;

    vos_task_sleep(100);

    vos_task_delete(&cb->task);
}

static void __lwm2m_destroy(lwm2m_cb_t *cb)
{
    __lwm2m_disconnect(cb);

    lwm2m_close(cb->lwm2m_context);

    lwm2m_rpt_destroy();

    if (cb->recv_buf != NULL) {
        lwm2m_free(cb->recv_buf);
    }

    lwm2m_free_object(cb->lwm2m_context);
    lwm2m_destroy_client_data(cb);

    vos_free(cb);
}

static int __task_entry(uintptr_t arg)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)arg;
    lwm2m_context_t *ctx = cb->lwm2m_context;
    uint32_t timeout;
    int ret;

    while (!cb->quit_flag)
    {
        timeout = BIND_TIMEOUT * 1000;

        lwm2m_rpt_step(ctx, lwm2m_rpt_notify);

        ret = lwm2m_step(ctx, (time_t *)&timeout);
        if (ret == COAP_503_SERVICE_UNAVAILABLE) {
            if (ctx->state == STATE_BOOTSTRAP_REQUIRED) {
                ctx->state = STATE_INITIAL;
            }
        }
        if (timeout < 1000) {
            timeout = 1000;
        }

        lwm2m_poll(cb, timeout);
    }
    __lwm2m_destroy(cb);
    return 0;
}

static int __init(uintptr_t *handle, lwm2m_al_config_t *config)
{
    lwm2m_cb_t *cb = NULL;
    int ret = -1;

    cb = (lwm2m_cb_t *)vos_zalloc(sizeof(lwm2m_cb_t));
    if (cb == NULL) {
        return -1;
    }

    cb->recv_buf = (uint8_t *)vos_zalloc(MAX_PACKET_SIZE);
    if (cb->recv_buf == NULL) {
        goto EXIT_FREE_CB;
    }

    if (lwm2m_rpt_init() != 0) {
        goto EXIT_FREE_BUF;
    }

    cb->lwm2m_context = lwm2m_init(&cb->client_data);
    if (cb->lwm2m_context == NULL) {
        goto EXIT_DESTROY_RPT;
    }
    cb->lwm2m_context->endpointName = config->endpoint;
    lwm2m_cmd_register_dealer(config->dealer);

    if ((ret = lwm2m_init_client_data(cb)) != 0) {
        vlog_error("lwm2m_init_client_data() failed, ret = %d", ret);
        goto EXIT_CLOSE_CONTEXT;
    }

    *handle = (uintptr_t)cb;

    return 0;

EXIT_CLOSE_CONTEXT:
    lwm2m_close(cb->lwm2m_context);
EXIT_DESTROY_RPT:
    lwm2m_rpt_destroy();
EXIT_FREE_BUF:
    vos_free(cb->recv_buf);
EXIT_FREE_CB:
    vos_free(cb);

    return -1;
}

static int __destroy(uintptr_t handle)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)handle;

    __lwm2m_destroy(cb);
    return 0;
}

static int __add_object(uintptr_t handle, lwm2m_al_uri_t *uri, uintptr_t obj_data)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)handle;

    return lwm2m_add_object_ex(cb->lwm2m_context, uri, obj_data);
}

static int __rm_object(uintptr_t handle, uint16_t obj_id)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)handle;

    return lwm2m_remove_object(cb->lwm2m_context, obj_id);
}

static int __connect(uintptr_t handle)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)handle;

    if (lwm2m_check_object(cb->lwm2m_context) != 0) {
        return -1;
    }
    cb->client_data.securityObjP =
        (lwm2m_object_t *)LWM2M_LIST_FIND(cb->lwm2m_context->objectList, LWM2M_SECURITY_OBJECT_ID);

    if (vos_task_create(&cb->task, "wakaama_lwm2m", __task_entry, (uintptr_t)cb, 0x1000, 10) != 0) {
        return -1;
    }
    return 0;
}

static int __disconnect(uintptr_t handle)
{
    lwm2m_cb_t *cb = (lwm2m_cb_t *)handle;

    __lwm2m_disconnect(cb);

    return 0;
}

static int __send(uintptr_t handle, const char *uri, const char *msg, int len, uint32_t timeout)
{
    lwm2m_uri_t lwm2m_uri;
    rpt_data_t data;

    if (lwm2m_stringToUri(uri, strlen(uri), &lwm2m_uri) == 0) {
        return -1;
    }

    data.buf = (uint8_t *)vos_malloc(len);
    if (data.buf == NULL) {
        return -1;
    }
    memcpy(data.buf, msg, len);
    data.len = len;
    data.cookie = 0;
    data.callback = lwm2m_rpt_ack_callback;

    if (lwm2m_rpt_enqueue_data(&lwm2m_uri, &data) != 0) {
        vos_free(data.buf);
        return -1;
    }
    return 0;
}

static lwm2m_al_ops_t s_lwm2m_ops = {
    .init = __init,
    .destroy = __destroy,
    .add_object = __add_object,
    .rm_object = __rm_object,
    .connect = __connect,
    .disconnect = __disconnect,
    .send = __send
};

int lwm2m_setup(void)
{
    return lwm2m_al_install(&s_lwm2m_ops);
}