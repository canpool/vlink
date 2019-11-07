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

ifeq ($(CONFIG_FS), y)
	FS_DIR = $(COMPONENTS_DIR)/fs
	include $(FS_DIR)/fs.mk
endif

ifeq ($(CONFIG_NET), y)
	NET_DIR = $(COMPONENTS_DIR)/net
	include $(NET_DIR)/net.mk
endif

ifeq ($(CONFIG_OS), y)
	OS_DIR = $(COMPONENTS_DIR)/os
	include $(OS_DIR)/os.mk
endif

ifeq ($(CONFIG_SECURITY), y)
	SECURITY_DIR = $(COMPONENTS_DIR)/security
	include $(SECURITY_DIR)/security.mk
endif

CONNECTIVITY_DIR = $(COMPONENTS_DIR)/connectivity
include $(CONNECTIVITY_DIR)/connectivity.mk

C_SOURCES += \
	${wildcard $(COMPONENTS_DIR)/log/*.c} \
	${wildcard $(COMPONENTS_DIR)/misc/*.c} \
	${wildcard $(COMPONENTS_DIR)/link/*.c}

C_INCLUDES += \
	-I $(COMPONENTS_DIR)/include \
	-I $(COMPONENTS_DIR)/log \
	-I $(COMPONENTS_DIR)/misc \
	-I $(COMPONENTS_DIR)/link
