# Copyright (c) [2020] vlink Team. All rights reserved.
# SPDX-License-Identifier: MulanPSL-2.0

# global variables
VPATH           = $(root_dir)
MAGIC           = $(root_dir)/scripts/make/magic.mk

outdir          = gnuc

target          = $(notdir $(PWD))

# default values for VERBOSE, LIST, DEBUG
V              ?= 0
L              ?= 0
D              ?= 0

# variables used for iterate sub dirs must be defined as ':='
# parse from $(root_dir)
subs           := $(abspath $(root_dir))
curdir         :=
proj           := $(abspath ./)

# all dir (not abspath) being built will be added to dirs
dirs           :=

cflags-global  :=
inc-global     := -I$(root_dir)/include -I$(proj)/config

ifneq ($(CONFIG_LINUX),y)
inc-global     += -I$(root_dir)/include/posix
endif

arch            = $(patsubst "%",%,$(CONFIG_ARCH))
soc             = $(curdir)/$(patsubst "%",%,$(CONFIG_SOC))

include        config/kconfig.mk
include        $(root_dir)/scripts/arch/$(arch)/def.mk
include        $(root_dir)/scripts/make/common.mk
include        $(MAGIC)

cflags-global  += $(cflags-arch)

################################################################################
# tool gcc rules
################################################################################

# src-$(dir) holds the filename with absolutely path
srcs           := $(patsubst $(root_dir)%,$(outdir)%,$(foreach d,$(dirs),$(src-$(d))))

objs           := $(srcs:.c=.o)
objs           := $(objs:.s=.o)

outdirs        := $(sort $(dir $(objs)))

ifeq ($(V),1)
    $(foreach v, $(sort $(filter src-% inc-% par-% cflags-%, $(.VARIABLES))), $(info $(v) = $($(v))))
    $(info objects = $(objs))
    $(info outdir = $(strip $(outdir)))
    $(info outdirs = $(outdirs))
endif

.PHONY : all
all : $(outdir)/$(target).bin $(outdir)/$(target).hex

$(outdir)/$(target).hex : $(outdir)/$(target).elf
	$(OBJCOPY) $< $@ -O ihex

$(outdir)/$(target).bin : $(outdir)/$(target).elf
	$(OBJCOPY) $< $@ -O binary

$(outdir)/$(target).elf : $(objs) $(ld_script)
	$(CC) $(filter %.o, $^) $(cflags-arch) $(lflags) -o $@
	$(SIZE) $@

$(outdir)/%.o : %.c
	$(strip $(CC) $(cflags-common) $(cflags-global) $(cflags-$<) -I$(proj) $(inc-global) $(inc-$(abspath $<)) $< -c -o $@)

$(outdir)/%.o : %.s
	$(strip $(CC) $(cflags-common) $(cflags-global) $(cflags-$<) -I$(proj) $(inc-global) $(inc-$(abspath $<)) $< -c -o $@)

.PHONY : clean
clean :
	@rm -rf $(objs) $(objs:.o=.d) $(objs:.o=.list)

# define rules for creating out dirs
define dir-rule
    ifeq ($(V),1)
        $$(info define rule for $(1))
    endif
    $(1) :
	@mkdir -p $(1)
endef

$(foreach d,$(outdirs),$(eval $(call dir-rule,$(d))))

# define depend rules for objs to dirs
define obj-rule
    ifeq ($(V),1)
        $$(info define rule for $(1) : $$(dir $(1)))
    endif
    $(1) : | $$(dir $(1))
endef

$(foreach o,$(objs),$(eval $(call obj-rule,$(o))))

-include $(objs:.o=.d)
