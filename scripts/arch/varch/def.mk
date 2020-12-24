# Copyright (c) [2020] vlink Team. All rights reserved.
# SPDX-License-Identifier: MulanPSL-2.0

GNU_PREFIX :=

ifdef CONFIG_VARCH_32
cflags-arch += -m32
else ifdef CONFIG_VARCH_64
cflags-arch += -m64
endif
