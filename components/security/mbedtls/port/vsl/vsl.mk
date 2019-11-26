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

MBEDTLS_VSL_SRC =

MBEDTLS_VSL_INC = \
        -I $(MBEDTLS_VSL_DIR)

__CONFIG_MBEDTLS_VSL__ := n

# hmac
ifeq ($(CONFIG_HMAC), y)
	ifeq ($(CONFIG_HMAC_TYPE), mbedtls)
	MBEDTLS_VSL_DEF += -D CONFIG_HMAC_MBEDTLS
	MBEDTLS_VSL_SRC += $(MBEDTLS_VSL_DIR)/vsl_hmac.c
	__CONFIG_MBEDTLS_VSL__ := y
	endif
endif

# curve25519
ifeq ($(CONFIG_CURVE25519), y)
	ifeq ($(CONFIG_CURVE25519_TYPE), mbedtls)
	MBEDTLS_VSL_DEF += -D CONFIG_CURVE25519_MBEDTLS
	MBEDTLS_VSL_SRC += $(MBEDTLS_VSL_DIR)/vsl_curve25519.c
	__CONFIG_MBEDTLS_VSL__ := y
	endif
endif

# ecdsa256
ifeq ($(CONFIG_ECDSA256), y)
	ifeq ($(CONFIG_ECDSA256_TYPE), mbedtls)
	MBEDTLS_VSL_DEF += -D CONFIG_ECDSA256_MBEDTLS
	MBEDTLS_VSL_SRC += $(MBEDTLS_VSL_DIR)/vsl_ecdsa256.c
	__CONFIG_MBEDTLS_VSL__ := y
	endif
endif

ifeq ($(__CONFIG_MBEDTLS_VSL__), y)
	include $(MBEDTLS_DIR)/core.mk

	C_SOURCES += $(MBEDTLS_VSL_SRC)
	C_INCLUDES += $(MBEDTLS_VSL_INC)
	C_DEFS += $(MBEDTLS_VSL_DEF)
endif
