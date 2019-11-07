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

ifeq ($(CONFIG_FS_TYPE), mockfs)
	MOCKFS_DIR = $(FS_DIR)/mockfs
	include $(MOCKFS_DIR)/mockfs.mk
else ifeq ($(CONFIG_FS_TYPE), fatfs)
	FATFS_DIR = $(FS_DIR)/fatfs
	include $(FATFS_DIR)/fatfs.mk
else ifeq ($(CONFIG_FS_TYPE), spiffs)
	SPIFFS_DIR = $(FS_DIR)/spiffs
	include $(SPIFFS_DIR)/spiffs.mk
else ifeq ($(CONFIG_FS_TYPE), jffs2)
	JFFS2_DIR = $(FS_DIR)/jffs2
	include $(JFFS2_DIR)/jffs2.mk
else ifeq ($(CONFIG_FS_TYPE), yaffs2)
	YAFFS2_DIR = $(FS_DIR)/yaffs2
	include $(YAFFS2_DIR)/yaffs2.mk
else ifeq ($(CONFIG_FS_TYPE), uffs)
	UFFS_DIR = $(FS_DIR)/uffs
	include $(UFFS_DIR)/uffs.mk
else
	$(error "please config fs type")
endif

ifneq ($(CONFIG_OS_TYPE), $(filter $(CONFIG_OS_TYPE), linux macos))
	C_INCLUDES += -I $(FS_DIR)/include
endif

VFS_DIR = $(FS_DIR)/vfs

VFS_SRC = ${wildcard $(VFS_DIR)/*.c}
C_SOURCES += $(VFS_SRC)

VFS_INC = -I $(VFS_DIR)
C_INCLUDES += $(VFS_INC)

C_DEFS += -D CONFIG_FS=1
