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

MBEDTLS_ECDSA_SRC = \
        ${wildcard $(MBEDTLS_ECDSA_DIR)/*.c}

MBEDTLS_ECDSA_INC = \
        -I $(MBEDTLS_ECDSA_DIR)

C_SOURCES += $(MBEDTLS_ECDSA_SRC)
C_INCLUDES += $(MBEDTLS_ECDSA_INC)

C_DEFS += -D MBEDTLS_CONFIG_FILE=\"mbedtls_config_ecdsa.h\"