######################################
# paths
######################################
CUR_DIR		= $(PWD)
TOP_DIR		= $(CUR_DIR)/../../../..
ROOT_DIR	= $(TOP_DIR)
TEST_DIR	= $(TOP_DIR)/tests
GTEST_DIR	= $(TEST_DIR)/3rd/googletest
LLT_DIR		= $(TEST_DIR)/llt
COMMON_DIR	= $(LLT_DIR)/fs/common
GCOV_DIR	= $(BUILD_DIR)

######################################
# config
######################################
FILE_PATTERN = "*jffs2*"

## config sources
CONFIG_OS			:= y
CONFIG_FS			:= y
CONFIG_OS_TYPE		:= linux
CONFIG_FS_TYPE 		:= jffs2

include $(ROOT_DIR)/vlink.mk

######################################
# source
######################################
# C sources
C_SOURCES += \
		$(wildcard $(CUR_DIR)/mock/*.c) \
		$(wildcard $(COMMON_DIR)/*.c)

# C++ sources
CPP_SOURCES += $(wildcard $(CUR_DIR)/src/*.cpp)

#######################################
# CFLAGS
#######################################
# C defines
C_DEFS	+=

# C includes
C_INCLUDES += \
		-I $(GTEST_DIR)/include \
		-I $(CUR_DIR)/mock \
		-I $(COMMON_DIR)

# C++ includes
CPP_INCLUDES += -I .

# library
LIB_DIR += -L$(CUR_DIR) -L$(GTEST_DIR)/lib

#######################################
# build the application
#######################################
# config
include $(LLT_DIR)/config.mk
# build
include $(LLT_DIR)/build.mk
