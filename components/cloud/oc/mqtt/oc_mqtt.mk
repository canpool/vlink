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

OC_MQTT_SRC = \
	${wildcard $(OC_MQTT_DIR)/*.c} \
	${wildcard $(OC_MQTT_DIR)/agent/*.c}
C_SOURCES += $(OC_MQTT_SRC)

OC_MQTT_INC = \
	-I $(OC_MQTT_DIR) \
	-I $(OC_MQTT_DIR)/agent
C_INCLUDES += $(OC_MQTT_INC)

C_DEFS += -D CONFIG_OC_MQTT=1