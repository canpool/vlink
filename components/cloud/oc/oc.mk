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

ifeq ($(CONFIG_CLOUD_PROTO_TYPE), coap)
	OC_COAP_DIR = $(OC_DIR)/coap
	include $(OC_COAP_DIR)/oc_coap.mk
else ifeq ($(CONFIG_CLOUD_PROTO_TYPE), mqtt)
	OC_MQTT_DIR = $(OC_DIR)/mqtt
	include $(OC_MQTT_DIR)/oc_mqtt.mk
else
	$(error "please config cloud protocol type")
endif
