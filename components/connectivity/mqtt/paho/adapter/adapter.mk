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

PAHO_MQTT_PORT_DIR = $(PAHO_MQTT_ADAPTER_DIR)/port

PAHO_MQTT_SRC += \
		${wildcard $(PAHO_MQTT_PORT_DIR)/*.c}

PAHO_MQTT_INC +=  \
		-I $(PAHO_MQTT_PORT_DIR)

ifeq ($(CONFIG_CLOUD), y)
	ifeq ($(CONFIG_CLOUD_TYPE), oc)
		ifeq ($(CONFIG_CLOUD_PROTO_TYPE), mqtt)
		PAHO_MQTT_SRC += ${wildcard $(PAHO_MQTT_ADAPTER_DIR)/oc/*.c}
		PAHO_MQTT_INC += -I $(PAHO_MQTT_ADAPTER_DIR)/oc
		endif
	endif
endif

C_DEFS += -D MQTTCLIENT_PLATFORM_HEADER=mqtt_os.h