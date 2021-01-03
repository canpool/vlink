ifdef CONFIG_GCOV
cflags-g-y      += -fprofile-arcs -ftest-coverage
lflags          += -lgcov

ifndef GCOV_FILE_PATTERN
tested_dirs := include link platform utils
GCOV_FILE_PATTERN = $(addsuffix /*",$(addprefix "$(root_dir)/,$(tested_dirs)))
endif
endif # CONFIG_GCOV

ifdef CONFIG_VALGRIND
cflags-memcheck = --tool=memcheck --leak-check=full --show-reachable=yes --trace-children=yes --smc-check=all \
                  --track-origins=yes
endif # CONFIG_VALGRIND
