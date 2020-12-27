dir-y       += platform link utils tests

inc-g-y     += include
ifneq ($(CONFIG_LINUX),y)
inc-g-y     += include/posix
endif
