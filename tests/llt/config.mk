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

###############################################################################
# common config file of Makefile
###############################################################################

CONFIG_FOR32 = y

CONFIG_GCOV_ENABLE = y
CONFIG_VALGRIND_ENABLE = y


# compile gcc flags
ifeq ($(CONFIG_FOR32), y)
CFLAGS += -m32 -march=i386 -mtune=i386
endif

CFLAGS += -g

# config gcov
ifeq ($(CONFIG_GCOV_ENABLE), y)
GCOV_FALGS = -fprofile-arcs -ftest-coverage -lgcov
CFLAGS += $(GCOV_FALGS)
endif

# config valgrind
ifeq ($(CONFIG_VALGRIND_ENABLE), y)
VALGRIND_FLAGS = --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes --smc-check=all
endif
