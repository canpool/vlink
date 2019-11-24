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

ifeq ($(CONFIG_LWM2M_TYPE), wakaama)
	WAKAAMA_DIR = $(LWM2M_DIR)/wakaama
	include $(WAKAAMA_DIR)/wakaama.mk
else
	$(error "please config lwm2m type")
endif

LWM2M_AL_DIR = $(LWM2M_DIR)/lwm2m_al
C_SOURCES += ${wildcard $(LWM2M_AL_DIR)/*.c}
C_INCLUDES += -I $(LWM2M_AL_DIR)
C_DEFS += -D CONFIG_LWM2M=1