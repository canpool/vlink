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
# target
######################################
BIN = test
LIB = libobject.a

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
# C defines
C_DEFS	+= -DCONFIG_VLOG

# compile gcc flags
CFLAGS += $(C_DEFS) $(C_INCLUDES) $(CPP_INCLUDES)

# library
LIBS += -lobject -lgtest -lpthread -lrt
LDFLAGS += $(LIB_DIR) $(LIBS)

#######################################
# build the application
#######################################
# objects
LIB_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(patsubst %.c, %.o, $(C_SOURCES))))
vpath %.c $(sort $(dir $(C_SOURCES)))

CPP_OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(patsubst %.cpp, %.o, $(CPP_SOURCES))))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

all:$(BUILD_DIR) $(LIB) $(BIN)

# build lib
$(LIB):$(LIB_OBJECTS)
#	$(CPP) -shared -o $@ $^
	$(AR) -r $@ $^

$(LIB_OBJECTS):$(BUILD_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

# build bin
$(BIN):$(CPP_OBJECTS)
	$(CPP) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CPP_OBJECTS):$(BUILD_DIR)/%.o:%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

distclean:clean
	rm -f *.bin

clean:
	rm -rf $(BUILD_DIR) $(BIN) gcov *.a *.htm *.log

run:
	./$(BIN)

cov:run
	@mkdir -p gcov
	lcov -c -o gcov/gcov.info -d $(GCOV_DIR) -b $(GCOV_DIR) --rc lcov_branch_coverage=1
	lcov -e gcov/gcov.info $(FILE_PATTERN) -o gcov/gcov.info --rc lcov_branch_coverage=1
	genhtml -o gcov/html gcov/gcov.info --rc genhtml_branch_coverage=1

memcheck:
	valgrind $(VALGRIND_FLAGS) ./$(BIN) 2>&1 | tee memcheck.log
