/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vmodule.h"
#include "vbug.h"

#ifdef CONFIG_CONSTRUCTOR

static vslist_t s_modules[VMODULE_MAX];

void vmodule_add(vmoudle_e group, vmodule_t *module)
{
    if (group < VMODULE_MAX) {
        vslist_add_tail(&s_modules[group], &module->node);
    }
}

void vmodule_init(void)
{
    int i, ret;
    for (i = 0; i < VMODULE_MAX; ++i) {
        vmodule_t *m = NULL;
        vslist_foreach_entry(m, &s_modules[i], vmodule_t, node) {
            if (m->init == NULL) {
                continue;
            }
            if ((ret = m->init()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) group(%d) init fail %d", m->init, i, ret);
            }
        }
    }
}

void vmodule_exit(void)
{
    int i, ret;
    for (i = VMODULE_MAX - 1; i >= 0; --i) {
        vmodule_t *m = NULL;
        vslist_foreach_entry(m, &s_modules[i], vmodule_t, node) {
            if (m->exit == NULL) {
                continue;
            }
            if ((ret = m->exit()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) group(%d) exit fail %d", m->exit, i, ret);
            }
        }
    }
}

#else

extern char v_section_start(vm_cpu) [];
extern char v_section_end  (vm_user)[];

static void __walk_init_table(vmodule_t *start, vmodule_t *end)
{
    int ret;

    while (start != end) {
        if (start->init != NULL) {
            if ((ret = start->init()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) init fail %d", start->init, ret);
            }
        }
        start++;
    }
}

static void __walk_exit_table(vmodule_t *start, vmodule_t *end)
{
    int ret;

    while (start != end) {
        if (end->exit != NULL) {
            if ((ret = end->exit()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) exit fail %d", end->exit, ret);
            }
        }
        end--;
    }
}

void vmodule_init(void)
{
    __walk_init_table((vmodule_t *)v_section_start(vm_cpu),
                      (vmodule_t *)v_section_end  (vm_user));
}

void vmodule_exit(void)
{
    __walk_exit_table((vmodule_t *)v_section_start(vm_cpu),
                      (vmodule_t *)v_section_end  (vm_user));
}

#endif // CONFIG_CONSTRUCTOR
