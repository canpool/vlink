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

MBEDTLS_BASE_DIR = $(MBEDTLS_DIR)/mbedtls-2.16.2
MBEDTLS_PORT_DIR = $(MBEDTLS_DIR)/port

MBEDTLS_SRC = \
        ${wildcard $(MBEDTLS_BASE_DIR)/library/*.c} \
        ${wildcard $(MBEDTLS_PORT_DIR)/vsl/*.c}

MBEDTLS_INC = \
        -I $(MBEDTLS_BASE_DIR)/include

ifeq ($(CONFIG_DTLS), y)
    MBEDTLS_DTLS_DIR = $(MBEDTLS_PORT_DIR)/dtls
    include $(MBEDTLS_DTLS_DIR)/dtls.mk
endif


ifeq ($(CONFIG_CURVE25519), y)
    MBEDTLS_CURVE_DIR = $(MBEDTLS_PORT_DIR)/curve25519
    include $(MBEDTLS_CURVE_DIR)/curve25519.mk
endif

ifeq ($(CONFIG_ECDSA256), y)
    MBEDTLS_ECDSA_DIR = $(MBEDTLS_PORT_DIR)/ecdsa256
    include $(MBEDTLS_ECDSA_DIR)/ecdsa256.mk
endif

C_SOURCES += $(MBEDTLS_SRC)
C_INCLUDES += $(MBEDTLS_INC)
C_DEFS += $(MBEDTLS_DEF)