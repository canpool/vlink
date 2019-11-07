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

ifeq ($(CONFIG_OS_TYPE), linux)
	LINUX_DIR = $(OS_DIR)/linux
	include $(LINUX_DIR)/linux.mk
else
	$(error "please config os type")
endif

VOS_DIR = $(OS_DIR)/vos

VOS_SRC = ${wildcard $(VOS_DIR)/*.c}
C_SOURCES += $(VOS_SRC)

VOS_INC = -I $(VOS_DIR)
C_INCLUDES += $(VOS_INC)

## sys include
ifneq ($(CONFIG_OS_TYPE), linux)
	C_INCLUDES += -I $(OS_DIR)/include
endif

C_DEFS += -D CONFIG_OS=1