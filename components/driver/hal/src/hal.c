/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "hal.h"
#include "vos.h"
#include "vmodule.h"

#include <string.h>

static vdlist_t s_devs = VDLIST_INIT(s_devs);
static vmutex_t s_dev_lock;

hal_dev_t * hal_find(const char *name)
{
    vdlist_t  * iter;
    hal_dev_t * dev = NULL;

    if (name == NULL) {
        return dev;
    }
    vdlist_foreach(iter, &s_devs) {
        dev = container_of(iter, hal_dev_t, node);
        if (strcmp(name, dev->name) == 0) {
            return dev;
        }
    }
    return NULL;
}

hal_dev_t * hal_open(const char *name, int oflag, ...)
{
    hal_dev_t * dev = NULL;
    va_list     args;

    if (name == NULL) {
        return NULL;
    }
    if (vmutex_lock(&s_dev_lock) != 0) {
        return NULL;
    }
    dev = hal_find(name);
    if (dev != NULL && dev->ops != NULL && dev->ops->open != NULL) {
        va_start(args, oflag);
        dev->ops->open(dev, oflag, args);
        va_end(args);
    }
    vmutex_unlock(&s_dev_lock);

    return dev;
}

int hal_close(hal_dev_t *dev)
{
    if (dev == NULL) {
        return VEINVAL;
    }
    if (dev->ops == NULL || dev->ops->close == NULL) {
        return VEPERM;
    }
    return dev->ops->close(dev);
}

size_t hal_read(hal_dev_t *dev, int pos, void *buf, size_t len)
{
    if (dev == NULL || buf == NULL || len == 0) {
        return VEINVAL;
    }
    if (dev->ops == NULL || dev->ops->read == NULL) {
        return VEPERM;
    }
    return dev->ops->read(dev, pos, buf, len);
}

size_t hal_write(hal_dev_t *dev, int pos, const void *buf, size_t len)
{
    if (dev == NULL || buf == NULL || len == 0) {
        return VEINVAL;
    }
    if (dev->ops == NULL || dev->ops->write == NULL) {
        return VEPERM;
    }
    return dev->ops->write(dev, pos, buf, len);
}

int hal_ioctl(hal_dev_t *dev, int cmd, ...)
{
    int     ret;
    va_list args;

    if (dev == NULL) {
        return VEINVAL;
    }
    if (dev->ops == NULL || dev->ops->ioctl == NULL) {
        return VEPERM;
    }
    va_start(args, cmd);
    ret = dev->ops->ioctl(dev, cmd, args);
    va_end(args);

    return ret;
}

int hal_init(void)
{
    return vmutex_init(&s_dev_lock);
}

int hal_exit(void)
{
    if (!vdlist_empty(&s_devs)) {
        return VEPERM;
    }
    return vmutex_destroy(&s_dev_lock);
}

VMODULE_DEF(hal, hal_init, hal_exit);

int hal_register(hal_dev_t *dev)
{
    if (dev == NULL || dev->name == NULL) {
        return VEINVAL;
    }
    if (vmutex_lock(&s_dev_lock) != 0) {
        return VEMUTEX;
    }
    if (hal_find(dev->name) != NULL) {
        vmutex_unlock(&s_dev_lock);
        return VEEXIST;
    }
    vdlist_add_tail(&s_devs, &dev->node);
    vmutex_unlock(&s_dev_lock);

    return VOK;
}

int hal_unregister(hal_dev_t *dev)
{
    if (dev == NULL) {
        return VEINVAL;
    }
    if (vdlist_uninitilized(&dev->node)) {
        return VEINVAL;
    }
    vdlist_del_init(&dev->node);

    return VOK;
}
