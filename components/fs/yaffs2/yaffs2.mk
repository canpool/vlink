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

YAFFS2_SRC = \
		${wildcard $(YAFFS2_DIR)/*.c} \
		${wildcard $(YAFFS2_DIR)/port/*.c} \
		${wildcard $(YAFFS2_DIR)/src/direct/*.c} \
		${wildcard $(YAFFS2_DIR)/src/*.c}

C_SOURCES += $(YAFFS2_SRC)

YAFFS2_INC = \
		-I $(YAFFS2_DIR) \
		-I $(YAFFS2_DIR)/port \
		-I $(YAFFS2_DIR)/src/direct \
		-I $(YAFFS2_DIR)/src
C_INCLUDES += $(YAFFS2_INC)

C_DEFS += -D CONFIG_YAFFS2=1
