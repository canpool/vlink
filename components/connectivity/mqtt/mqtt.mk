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

ifeq ($(CONFIG_MQTT_TYPE), paho)
	PAHO_DIR = $(MQTT_DIR)/paho
	include $(PAHO_DIR)/paho.mk
else
	$(error "please config mqtt type")
endif

MQTT_AL_DIR = $(MQTT_DIR)/mqtt_al
C_SOURCES += ${wildcard $(MQTT_AL_DIR)/*.c}
C_INCLUDES += -I $(MQTT_AL_DIR)
C_DEFS += -D CONFIG_MQTT=1