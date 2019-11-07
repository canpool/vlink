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

######################################
# paths
######################################
BUILD_DIR	= build

######################################
# binaries
######################################
CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
LD	= $(PREFIX)ld
AR	= $(PREFIX)ar

#######################################
# CFLAGS
#######################################
# library
LIBS += -lpthread -lrt
LDFLAGS += $(LIB_DIR) $(LIBS)

#######################################
# build the application
#######################################
# objects
C_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(patsubst %.c, %.o, $(C_SOURCES))))
vpath %.c $(sort $(dir $(C_SOURCES)))

all:$(BUILD_DIR) $(BIN)

# build bin
$(BIN):$(C_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(C_OBJECTS):$(BUILD_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

distclean:clean

clean:
	rm -rf $(BUILD_DIR) $(BIN) *.txt

run:
	./$(BIN)