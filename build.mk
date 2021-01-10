dir-y       += platform link utils components tests

inc-g-y     += include
ifneq ($(CONFIG_LINUX),y)
inc-g-y     += include/posix
endif
