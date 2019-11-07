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

##
# this compat/dirent.h is used to replace dirent.h in Linux/macos
##
ifeq ($(CONFIG_OS_TYPE), $(filter $(CONFIG_OS_TYPE), linux macos))
	C_INCLUDES += -I $(FS_DIR)/include/compat
endif

SPIFFS_BASE_DIR     = $(SPIFFS_DIR)/spiffs-0.3.7/src

SPIFFS_SRC = \
		${wildcard $(SPIFFS_BASE_DIR)/*.c} \
		${wildcard $(SPIFFS_DIR)/*.c}

C_SOURCES += $(SPIFFS_SRC)

SPIFFS_INC = \
		-I $(SPIFFS_BASE_DIR) \
		-I $(SPIFFS_DIR)
C_INCLUDES += $(SPIFFS_INC)

C_DEFS += -D CONFIG_SPIFFS=1
