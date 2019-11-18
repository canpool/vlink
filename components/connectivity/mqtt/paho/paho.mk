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

PAHO_MQTT_BASE_DIR 	 = $(PAHO_DIR)/paho.mqtt.embedded-c

PAHO_MQTT_CLIENT_DIR = $(PAHO_MQTT_BASE_DIR)/MQTTClient-C/src
PAHO_MQTT_PACKET_DIR = $(PAHO_MQTT_BASE_DIR)/MQTTPacket/src

PAHO_MQTT_SRC = \
		${wildcard $(PAHO_MQTT_CLIENT_DIR)/*.c} \
		${wildcard $(PAHO_MQTT_PACKET_DIR)/*.c}

PAHO_MQTT_INC = \
		-I $(PAHO_MQTT_CLIENT_DIR)	\
		-I $(PAHO_MQTT_PACKET_DIR)

# adapter
PAHO_MQTT_ADAPTER_DIR = $(PAHO_DIR)/adapter
include $(PAHO_MQTT_ADAPTER_DIR)/adapter.mk

C_SOURCES  += $(PAHO_MQTT_SRC)
C_INCLUDES += $(PAHO_MQTT_INC)