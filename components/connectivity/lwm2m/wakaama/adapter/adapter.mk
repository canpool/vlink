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

WAKAAMA_PORT_DIR   = $(WAKAAMA_ADAPTER_DIR)/port
WAKAAMA_SHARED_DIR = $(WAKAAMA_ADAPTER_DIR)/shared

WAKAAMA_SRC += \
		${wildcard $(WAKAAMA_PORT_DIR)/*.c} \
		${wildcard $(WAKAAMA_SHARED_DIR)/*.c}

WAKAAMA_INC +=  \
		-I $(WAKAAMA_PORT_DIR) \
		-I $(WAKAAMA_SHARED_DIR)

ifeq ($(CONFIG_CLOUD), y)
	ifeq ($(CONFIG_CLOUD_TYPE), oc)
		ifeq ($(CONFIG_CLOUD_PROTO_TYPE), lwm2m)
		WAKAAMA_SRC += ${wildcard $(WAKAAMA_ADAPTER_DIR)/oc/*.c}
		WAKAAMA_INC += -I $(WAKAAMA_ADAPTER_DIR)/oc
		endif
	endif
endif