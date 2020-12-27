# Copyright (c) [2020] vlink Team. All rights reserved.
# SPDX-License-Identifier: MulanPSL-2.0

include $(root_dir)/scripts/make/entry.mk

run:
	./$(outdir)/$(target).elf

ifdef CONFIG_GCOV
cov:run
	@mkdir -p gcov
	lcov -c -o gcov/gcov.info -d $(outdir) -b $(outdir) --rc lcov_branch_coverage=1
	lcov -e gcov/gcov.info $(GCOV_FILE_PATTERN) -o gcov/gcov.info --rc lcov_branch_coverage=1
	genhtml -o gcov/html gcov/gcov.info --rc genhtml_branch_coverage=1
endif

ifdef CONFIG_VALGRIND
memcheck:
	valgrind $(cflags-memcheck) ./$(outdir)/$(target).elf 2>&1 | tee memcheck.log
endif
