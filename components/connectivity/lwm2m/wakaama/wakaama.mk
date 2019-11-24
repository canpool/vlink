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

WAKAAMA_BASE_DIR 	 = $(WAKAAMA_DIR)/wakaama-master/core

WAKAAMA_SRC = \
		${wildcard $(WAKAAMA_BASE_DIR)/*.c} \
		${wildcard $(WAKAAMA_BASE_DIR)/er-coap-13/*.c}

WAKAAMA_INC = \
		-I $(WAKAAMA_BASE_DIR)

# adapter
WAKAAMA_ADAPTER_DIR = $(WAKAAMA_DIR)/adapter
include $(WAKAAMA_ADAPTER_DIR)/adapter.mk

C_SOURCES  += $(WAKAAMA_SRC)
C_INCLUDES += $(WAKAAMA_INC)

C_DEFS += -D LWM2M_LITTLE_ENDIAN -D LWM2M_CLIENT_MODE