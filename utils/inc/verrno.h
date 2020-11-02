/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VERRNO_H__
#define __VERRNO_H__

#include "sys/errno.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum VERRNO {
/* universal errno */
    VOK             =    0,         /* Success */
    VERR            =   -1,         /* No reason error */
    VEINVAL         =   -2,         /* Invalid argument */
    VENOMEM         =   -3,         /* Out of memory */
    VEPERM          =   -4,         /* Operation not permitted */
    VEFAULT         =   -5,         /* Bad address */
    VEEXIST         =   -6,         /* File or content exists */
    VEEMPTY         =   -7,         /* Read empty*/
    VEFULL          =   -8,         /* Write full */

/* operation system errno */
    VETASK          =   -100,       /* Task error */
    VESEM           =   -101,       /* Semaphore error */
    VEMUTEX         =   -102,       /* Mutex error */
    VEEVENT         =   -103,       /* Event error */
    VEMQ            =   -104,       /* Message queue error */
    VETIMER         =   -105,       /* Timer error */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VERRNO_H__ */
