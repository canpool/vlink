######################################
# paths
######################################
CUR_DIR     = $(PWD)
TOP_DIR     = $(CUR_DIR)/../../../..
ROOT_DIR    = $(TOP_DIR)
TEST_DIR    = $(TOP_DIR)/tests
GTEST_DIR   = $(TEST_DIR)/3rd/googletest
LLT_DIR     = $(TEST_DIR)/llt
GCOV_DIR    = $(BUILD_DIR)

######################################
# config
######################################
FILE_PATTERN = "*hash*" "*cmac*" "*pbkdf2*"

## config sources
CONFIG_OS               := y
CONFIG_OS_TYPE          := linux

CONFIG_HASH             := y
CONFIG_HASH_TYPE        := mbedtls

CONFIG_CMAC             := y
CONFIG_CMAC_TYPE        := mbedtls

CONFIG_PBKDF2         	:= y
CONFIG_PBKDF2_TYPE    	:= mbedtls

include $(ROOT_DIR)/vlink.mk

######################################
# source
######################################
# C sources
C_SOURCES +=

# C++ sources
CPP_SOURCES += $(wildcard $(CUR_DIR)/src/*.cpp)

#######################################
# CFLAGS
#######################################
# C defines
C_DEFS	+=

# C includes
C_INCLUDES += \
		-I $(GTEST_DIR)/include

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
