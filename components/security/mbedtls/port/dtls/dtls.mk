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

MBEDTLS_DTLS_SRC = \
        ${wildcard $(MBEDTLS_DTLS_DIR)/*.c}

MBEDTLS_DTLS_INC = \
        -I $(MBEDTLS_DTLS_DIR)

MBEDTLS_DTLS_DEF = -D WITH_DTLS

ifeq ($(CONFIG_DTLS_TYPE), psk)
    MBEDTLS_DTLS_DEF += -D MBEDTLS_CONFIG_FILE=\"mbedtls_config_psk.h\"
else
	$(error "please config dtls type")
endif

C_SOURCES += $(MBEDTLS_DTLS_SRC)
C_INCLUDES += $(MBEDTLS_DTLS_INC)
C_DEFS += $(MBEDTLS_DTLS_DEF)