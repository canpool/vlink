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

CONFIG_OS                   := y
CONFIG_OS_TYPE              := linux

CONFIG_NET                  := y
CONFIG_NET_TYPE             := linux

CONFIG_COAP                 := y
CONFIG_COAP_TYPE            := libcoap

CONFIG_CLOUD                := y
CONFIG_CLOUD_TYPE           := oc
CONFIG_CLOUD_PROTO_TYPE     := coap

CONFIG_SECURITY				:= n
CONFIG_SECURITY_TYPE		:= mbedtls
CONFIG_DTLS					:= y
CONFIG_DTLS_TYPE			:= psk

# oc_coap_demo, oc_dtls_coap_demo
CONFIG_DEMO                 := y
CONFIG_DEMO_TYPE            := oc_coap_demo

include $(ROOT_DIR)/vlink.mk