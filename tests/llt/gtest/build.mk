dir-$(CONFIG_GTEST) = *

inc-l-y = ../../3rd/googletest/include

# common config

CX              := $(CPP)

gtest_dir       = $(root_dir)/tests/3rd/googletest

ifeq ($(CONFIG_ARCH_BITS),"32")
lib_dirs        += $(gtest_dir)/lib32
else
lib_dirs		+= $(gtest_dir)/lib64
endif

lflags          += -L$(lib_dirs) -lgtest
