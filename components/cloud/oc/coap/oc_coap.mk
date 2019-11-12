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

OC_COAP_SRC = \
	${wildcard $(OC_COAP_DIR)/*.c} \
	${wildcard $(OC_COAP_DIR)/agent/*.c}
C_SOURCES += $(OC_COAP_SRC)

OC_COAP_INC = \
	-I $(OC_COAP_DIR) \
	-I $(OC_COAP_DIR)/agent
C_INCLUDES += $(OC_COAP_INC)

C_DEFS += -D CONFIG_OC_COAP=1