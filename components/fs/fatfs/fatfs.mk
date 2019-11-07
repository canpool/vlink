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

FATFS_BASE_DIR 		= $(FATFS_DIR)/ff13c/source

FATFS_SRC = \
		${wildcard $(FATFS_BASE_DIR)/*.c} \
		${wildcard $(FATFS_DIR)/port/*.c} \
		${wildcard $(FATFS_DIR)/*.c}

C_SOURCES += $(FATFS_SRC)

FATFS_INC = \
		-I $(FATFS_BASE_DIR) \
		-I $(FATFS_DIR)/port \
		-I $(FATFS_DIR)
C_INCLUDES += $(FATFS_INC)

C_DEFS += -D CONFIG_FATFS=1
