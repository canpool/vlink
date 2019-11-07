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

UFFS_BASE_DIR = $(UFFS_DIR)/uffs-v1.3.6/src

UFFS_SRC = \
		${wildcard $(UFFS_DIR)/*.c} \
		${wildcard $(UFFS_DIR)/port/*.c} \
		${wildcard $(UFFS_BASE_DIR)/uffs/*.c}

C_SOURCES += $(UFFS_SRC)

UFFS_INC = \
		-I $(UFFS_DIR) \
		-I $(UFFS_DIR)/port \
		-I $(UFFS_BASE_DIR)/inc
C_INCLUDES += $(UFFS_INC)

C_DEFS += -D CONFIG_UFFS=1
