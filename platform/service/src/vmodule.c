/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vmodule.h"
#include "vbug.h"

static vslist_t s_modules[VMODULE_MAX];

void vmodule_add(vmoudle_e type, vmodule_t *module)
{
    if (type < VMODULE_MAX) {
        vslist_add_tail(&s_modules[type], &module->list);
    }
}

void vmodule_init(void)
{
    int i, ret;
    for (i = 0; i < VMODULE_MAX; ++i) {
        vmodule_t *m = NULL;
        vslist_foreach_entry(m, &s_modules[i], vmodule_t, list) {
            if (m->init == NULL) {
                continue;
            }
            if ((ret = m->init()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) type(%d) init fail %d", m->init, i, ret);
            }
        }
    }
}

void vmodule_exit(void)
{
    int i, ret;
    for (i = VMODULE_MAX - 1; i >= 0; --i) {
        vmodule_t *m = NULL;
        vslist_foreach_entry(m, &s_modules[i], vmodule_t, list) {
            if (m->exit == NULL) {
                continue;
            }
            if ((ret = m->exit()) != 0) {
                // look up the address value in the map file
                VWARN("module routine(%p) type(%d) exit fail %d", m->exit, i, ret);
            }
        }
    }
}
