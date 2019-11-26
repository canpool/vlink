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

__CONFIG_MBEDTLS_DTLS__ := y

MBEDTLS_DTLS_SRC = \
        ${wildcard $(MBEDTLS_DTLS_DIR)/*.c}

MBEDTLS_DTLS_INC = \
        -I $(MBEDTLS_DTLS_DIR)

MBEDTLS_DTLS_DEF = -D WITH_DTLS -D CONFIG_DTLS_MBEDTLS

ifeq ($(CONFIG_DTLS_TYPE), mbedtls_psk)
    MBEDTLS_DTLS_DEF += -D CONFIG_DTLS_MBEDTLS_PSK
else ifeq ($(CONFIG_DTLS_TYPE), mbedtls_cert)
    MBEDTLS_DTLS_DEF += -D CONFIG_DTLS_MBEDTLS_CERT
else ifeq ($(CONFIG_DTLS_TYPE), mbedtls_psk_cert)
    MBEDTLS_DTLS_DEF += -D CONFIG_DTLS_MBEDTLS_PSK -D CONFIG_DTLS_MBEDTLS_CERT
else
    $(warning "warning: dtls type is $(CONFIG_DTLS_TYPE)")
    __CONFIG_MBEDTLS_DTLS__ := n
endif

ifeq ($(__CONFIG_MBEDTLS_DTLS__), y)
    include $(MBEDTLS_DIR)/core.mk

    C_SOURCES += $(MBEDTLS_DTLS_SRC)
    C_INCLUDES += $(MBEDTLS_DTLS_INC)
    C_DEFS += $(MBEDTLS_DTLS_DEF)
endif