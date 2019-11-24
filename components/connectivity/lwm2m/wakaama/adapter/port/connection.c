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

#include "connection.h"

#include <ctype.h>
#include <stdint.h>

#include "object_comm.h"

#include "sal.h"
#include "vos.h"

#ifdef WITH_DTLS
#include "dtls.h"
#endif


static int parse_host_port(char *uri, char **ohost, char **oport)
{
    char *host;
    char *port;
    char *default_port;

    // parse uri in the form "coaps://[host]:[port]"
    if (strncmp(uri, "coaps://", strlen("coaps://")) == 0) {
        host = uri + strlen("coaps://");
        default_port = LWM2M_DTLS_PORT_STR;
    } else if (strncmp(uri, "coap://",  strlen("coap://")) == 0) {
        host = uri + strlen("coap://");
        default_port = LWM2M_STANDARD_PORT_STR;
    } else {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    port = strrchr(host, ':');
    if (port == NULL) {
        port = default_port;
    } else {
        // remove brackets
        if (host[0] == '[') {
            host++;
            if (*(port - 1) == ']') {
                *(port - 1) = 0;
            } else {
                return COAP_500_INTERNAL_SERVER_ERROR;
            }
        }
        // split strings
        *port = 0;
        port++;
    }

    *ohost = host;
    *oport = port;

    return COAP_NO_ERROR;
}

#ifdef WITH_DTLS
static int __dtls_connect(connection_t *connP, security_instance_t *targetP,
                          const char *host, const char *port)
{
    int ret;
    dtls_config_info config_info;
    dtls_handshake_info handshake_info;
    dtls_context *context = NULL;

    memset(&config_info, 0, sizeof(dtls_config_info));
    memset(&handshake_info, 0, sizeof(dtls_handshake_info));

    config_info.auth_type |= DTLS_AUTH_PSK;
    config_info.proto = MBEDTLS_NET_PROTO_UDP;
    config_info.endpoint = MBEDTLS_SSL_IS_CLIENT;
    config_info.pers = "lwm2m_dtls_client";
    config_info.psk.psk_key = (unsigned char *)targetP->secretKey;
    config_info.psk.psk_key_len = targetP->secretKeyLen;
    config_info.psk.psk_identity = (unsigned char *)targetP->publicIdentity;
    config_info.psk.psk_identity_len = targetP->publicIdLen;

    if ((context = dtls_create(&config_info)) == NULL) {
        return -1;
    }
    if ((ret = dtls_connect(context, host, port)) != 0) {
        goto EXIT_FAIL;
    }
    handshake_info.timeout = CONFIG_DTLS_HANDSHAKE_TIMEOUT;
    if ((ret = dtls_handshake(context, &handshake_info)) != 0) {
        goto EXIT_FAIL;
    }

    connP->sock = (uintptr_t)context;

    return 0;

EXIT_FAIL:
    dtls_destroy(context);
    return -1;
}
#endif

static int __socket_connect(connection_t *connP, const char *host, const char *port)
{
    int fd = sal_sockconnect(AF_INET, SOCK_DGRAM, 0, host, port);

    if (fd < 0) {
        return -1;
    }
    connP->sock = (uintptr_t)fd;

    return 0;
}

connection_t *connection_create(connection_t *connList,
                                lwm2m_object_t *securityObj,
                                int instanceId,
                                lwm2m_context_t *lwm2mH,
                                bool isServer)
{
    connection_t *connP = NULL;
    char *uri = NULL;
    char *host;
    char *port;
    security_instance_t *targetP;

    targetP = (security_instance_t *)LWM2M_LIST_FIND(securityObj->instanceList, instanceId);
    if (targetP == NULL || targetP->uri == NULL) {
        return NULL;
    }

    if (isServer) {
        // TODO: should we support server ?
        LOG("don't support server now");
        return NULL;
    } else {
        uri = lwm2m_strdup(targetP->uri);
        if (uri == NULL) {
            goto EXIT_FAIL;
        }
        if (parse_host_port(uri, &host, &port) != COAP_NO_ERROR) {
            goto EXIT_FAIL;
        }
    }

    connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP == NULL)  {
        goto EXIT_FAIL;
    }
    memset(connP, 0, sizeof(connection_t));

#ifdef WITH_DTLS
    if (targetP->securityMode != LWM2M_SECURITY_MODE_NONE) {
        if (__dtls_connect(connP, targetP, host, port) != 0) {
            goto EXIT_FAIL;
        }
        connP->dtls_flag = 1;
    } else
#endif
    {
        if (__socket_connect(connP, host, port) != 0) {
            goto EXIT_FAIL;
        }
        connP->dtls_flag = 0;
    }

    connP->next = connList;
    connP->securityObj = securityObj;
    connP->securityInstId = instanceId;
    connP->lwm2mH = lwm2mH;

    lwm2m_free(uri);
    return connP;

EXIT_FAIL:
    safe_free(uri);
    safe_free(connP);

    return NULL;
}

static void connection_close(connection_t *targetP)
{
#ifdef WITH_DTLS
    if (targetP->dtls_flag) {
        dtls_destroy((dtls_context *)targetP->sock);
    } else
#endif
    {
        sal_closesocket((int)targetP->sock);
    }
}

void connection_free(connection_t *connList)
{
    while (connList != NULL)
    {
        connection_t *nextP = connList->next;

        connection_close(connList);
        lwm2m_free(connList);

        connList = nextP;
    }
}

void *lwm2m_connect_server(uint16_t secObjInstID, void * userData)
{
    client_data_t   *dataP = (client_data_t *)userData;
    lwm2m_list_t    *instance;
    connection_t    *newConnP = NULL;
    lwm2m_object_t  *securityObj = dataP->securityObjP;

    instance = LWM2M_LIST_FIND(dataP->securityObjP->instanceList, secObjInstID);
    if (instance == NULL) return NULL;

    newConnP = connection_create(dataP->connList, securityObj, instance->id, dataP->lwm2mH, false);
    if (newConnP == NULL) {
        LOG("Connection creation failed");
        return NULL;
    }
    dataP->connList = newConnP;

    return (void *)newConnP;
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    int ret;
    connection_t *connP = (connection_t *)sessionH;

    (void)userdata; /* unused */

    if (connP == NULL) {
        LOG_ARG("#> failed sending %lu bytes, missing connection", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

#ifdef WITH_DTLS
    if (connP->dtls_flag) {
        ret = dtls_write((dtls_context *)connP->sock, buffer, length);
    } else
#endif
    {
        ret = sal_send((int)connP->sock, buffer, length, 0);
    }
    if (ret < 0) {
        LOG_ARG("#> failed sending %lu bytes", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    return COAP_NO_ERROR;
}

// timeout: ms
int lwm2m_buffer_recv(void *sessionH, uint8_t *buffer, size_t length, uint32_t timeout)
{
    connection_t *connP = (connection_t *)sessionH;
    int ret = -1;

#ifdef WITH_DTLS
    if (connP->dtls_flag == true)  {
        // security
        ret = dtls_read((dtls_context *)connP->sock, (char *)buffer, length, timeout);
    } else
#endif
    {
        ret = sal_timedrecv((int)connP->sock, buffer, length, 0, timeout);
    }

    return ret;
}

bool lwm2m_session_is_equal(void *session1, void *session2, void *userData)
{
    return (session1 == session2);
}

void lwm2m_close_connection(void *sessionH, void *userData)
{
    client_data_t *app_data;
    connection_t *targetP;

    app_data = (client_data_t *)userData;
    targetP = (connection_t *)sessionH;
    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        connection_close(targetP);
        lwm2m_free(targetP);
    }
    else
    {
        connection_t *parentP;

        parentP = app_data->connList;

        while (parentP != NULL && parentP->next != targetP)
        {
            parentP = parentP->next;
        }

        if (parentP != NULL)
        {
            parentP->next = targetP->next;
            connection_close(targetP);
            lwm2m_free(targetP);
        }
    }

    return;
}