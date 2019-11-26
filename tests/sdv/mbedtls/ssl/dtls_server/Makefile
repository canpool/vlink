######################################
# target
######################################
BIN = server

######################################
# paths
######################################
CUR_DIR		= $(PWD)
TOP_DIR		= $(CUR_DIR)/../../../../..
ROOT_DIR	= $(TOP_DIR)
TEST_DIR	= $(TOP_DIR)/tests
SDV_DIR		= $(TEST_DIR)/sdv

######################################
# config
######################################

## config sources
CONFIG_OS				:= y
CONFIG_OS_TYPE			:= linux

CONFIG_NET				:= y
CONFIG_NET_TYPE			:= linux

CONFIG_DTLS				:= y
CONFIG_DTLS_TYPE		:= mbedtls_psk

include $(ROOT_DIR)/vlink.mk

######################################
# source
######################################
# C sources
C_SOURCES += ${wildcard $(CUR_DIR)/*.c}

# C++ sources
CPP_SOURCES +=

#######################################
# CFLAGS
#######################################
# C defines
C_DEFS	+= -DMBEDTLS_SSL_SRV_C

# C includes
C_INCLUDES +=

# C++ includes
CPP_INCLUDES += -I .

# compile gcc flags
CFLAGS += $(C_DEFS) $(C_INCLUDES)

# library
LIB_DIR += -L$(CUR_DIR)

#######################################
# build the application
#######################################
include $(SDV_DIR)/build.mk

