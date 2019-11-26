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

ifeq ($(CONFIG_DTLS), y)
    MBEDTLS_DTLS_DIR = $(MBEDTLS_PORT_DIR)/dtls
    include $(MBEDTLS_DTLS_DIR)/dtls.mk
endif

MBEDTLS_VSL_DIR = $(MBEDTLS_PORT_DIR)/vsl
include $(MBEDTLS_VSL_DIR)/vsl.mk
