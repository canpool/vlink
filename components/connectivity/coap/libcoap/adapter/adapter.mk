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

LIBCOAP_PORT_DIR = $(LIBCOAP_ADAPTER_DIR)/port

LIBCOAP_SRC += \
		${wildcard $(LIBCOAP_PORT_DIR)/*.c}

LIBCOAP_INC +=  \
		-I $(LIBCOAP_PORT_DIR)

ifeq ($(CONFIG_CLOUD), y)
	ifeq ($(CONFIG_CLOUD_TYPE), oc)
		ifeq ($(CONFIG_CLOUD_PROTO_TYPE), coap)
		LIBCOAP_SRC += ${wildcard $(LIBCOAP_ADAPTER_DIR)/oc/*.c}
		LIBCOAP_INC += -I $(LIBCOAP_ADAPTER_DIR)/oc
		endif
	endif
endif