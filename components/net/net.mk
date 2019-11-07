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

ifeq ($(CONFIG_NET_TYPE), linux)
	NET_LINUX_DIR = $(NET_DIR)/linux
	include $(NET_LINUX_DIR)/linux.mk
else
	$(error "please config net type")
endif

## sys netinet
ifneq ($(CONFIG_OS_TYPE), $(filter $(CONFIG_OS_TYPE), linux macos))
	C_INCLUDES += -I $(NET_DIR)/include
endif

SAL_DIR = $(NET_DIR)/sal

SAL_SRC = ${wildcard $(SAL_DIR)/*.c}
C_SOURCES += $(SAL_SRC)

SAL_INC = -I $(SAL_DIR)
C_INCLUDES += $(SAL_INC)

C_DEFS += -D CONFIG_NET=1
