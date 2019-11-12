#/**
# * Copyright (c) [2019] maminjie <canpool@163.com>
# *
# * vlink is licensed under the Mulan PSL v1.
# * You can use this software according to the terms and conditions of the Mulan PSL v1.
# * You may obtain a copy of Mulan PSL v1 at:
# *
# *    http://license.coscl.org.cn/MulanPSL
# *
# * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
# * FIT FOR A PARTICULAR PURPOSE.
# * See the Mulan PSL v1 for more details.
# */

LIBCOAP_BASE_DIR    = $(LIBCOAP_DIR)/libcoap-4.2.0

LIBCOAP_SRC_DIR = $(LIBCOAP_BASE_DIR)/src

LIBCOAP_SRC = \
		${wildcard $(LIBCOAP_SRC_DIR)/address.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/async.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/block.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_debug.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_event.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_hashkey.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_session.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/encode.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/net.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/option.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/pdu.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/resource.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/str.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/subscribe.c} \
		${wildcard $(LIBCOAP_SRC_DIR)/uri.c}

ifeq ($(CONFIG_SECURITY_TYPE), tinydtls)
C_DEFS += -D HAVE_LIBTINYDTLS
LIBCOAP_SRC += \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_tinydtls.c}
else ifeq ($(CONFIG_SECURITY_TYPE), openssl)
C_DEFS += -D HAVE_OPENSSL
LIBCOAP_SRC += \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_openssl.c}
else ifeq ($(CONFIG_SECURITY_TYPE), gnutls)
C_DEFS += -D HAVE_LIBGNUTLS
LIBCOAP_SRC += \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_gnutls.c}
else
LIBCOAP_SRC += \
		${wildcard $(LIBCOAP_SRC_DIR)/coap_notls.c}
endif

LIBCOAP_INC = \
		-I $(LIBCOAP_BASE_DIR) \
		-I $(LIBCOAP_BASE_DIR)/include \
		-I $(LIBCOAP_BASE_DIR)/include/coap2

# adapter
LIBCOAP_ADAPTER_DIR	= $(LIBCOAP_DIR)/adapter
include $(LIBCOAP_ADAPTER_DIR)/adapter.mk

C_SOURCES += $(LIBCOAP_SRC)
C_INCLUDES += $(LIBCOAP_INC)

C_DEFS += -D CONFIG_COAP_LIBCOAP=1