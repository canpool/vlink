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

ifneq ($(__CONFIG_MBEDTLS__), y)
__CONFIG_MBEDTLS__ = y

MBEDTLS_SRC = \
        ${wildcard $(MBEDTLS_BASE_DIR)/library/*.c} \
        ${wildcard $(MBEDTLS_PORT_DIR)/shared/*.c}

MBEDTLS_INC = \
        -I $(MBEDTLS_BASE_DIR)/include \
        -I $(MBEDTLS_PORT_DIR)/config

MBEDTLS_DEF += -D MBEDTLS_CONFIG_FILE=\"mbedtls_config.h\"

C_SOURCES += $(MBEDTLS_SRC)
C_INCLUDES += $(MBEDTLS_INC)
C_DEFS += $(MBEDTLS_DEF)

endif # __CONFIG_MBEDTLS__
