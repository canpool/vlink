/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VSH_H__
#define __VSH_H__

#include "vconfig.h"
#include "vpool.h"
#include "vring.h"
#include "vlist.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedefs */

/* ascii */
enum vsh_vk {
    vsh_vk_nul = 0x00,
    vsh_vk_ctrla,
    vsh_vk_ctrlb,
    vsh_vk_ctrlc,
    vsh_vk_ctrld,
    vsh_vk_ctrle,
    vsh_vk_ctrlf,
    vsh_vk_ctrlg,
    vsh_vk_ctrlh,
    vsh_vk_ctrli,
    vsh_vk_ctrlj,
    vsh_vk_ctrlk,
    vsh_vk_ctrll,
    vsh_vk_ctrlm,
    vsh_vk_ctrln,
    vsh_vk_ctrlo,
    vsh_vk_ctrlp,
    vsh_vk_ctrlq,
    vsh_vk_ctrlr,
    vsh_vk_ctrls,
    vsh_vk_ctrlt,
    vsh_vk_ctrlu,
    vsh_vk_ctrlv,
    vsh_vk_ctrlw,
    vsh_vk_ctrlx,
    vsh_vk_ctrly,
    vsh_vk_ctrlz,

    vsh_vk_esc,
    vsh_vk_fs,
    vsh_vk_gs,
    vsh_vk_rs,
    vsh_vk_us,

    /* printable, 0x20~0x7e */

    vsh_vk_del = 0x7f,

    vsh_vk_home,
    vsh_vk_end,
    vsh_vk_up,
    vsh_vk_down,
    vsh_vk_left,
    vsh_vk_right,
};

typedef struct vsh {
    char             input_buff[CONFIG_VSH_BUF_SIZE];
    uint8_t          input_idx;
    uint8_t          compl_idx;
    uint8_t          shift_idx;
    uint8_t          position;
    uint8_t          line_end;
    vring_t          his_cmd;
    vring_t          his_idx;
    unsigned char (* getc)(uintptr_t);
    int           (* putc)(uintptr_t, unsigned char);
    uintptr_t        arg;       /* argument for those routines */
} vsh_t;

/* macros */

#ifdef CONFIG_CONSTRUCTOR

#define __VSH_CMD_DEF__(cmd, tag)                                       \
    void __attribute__((constructor, used))                             \
    V_CONCAT(__vsh_, V_CONCAT(cmd, tag))(void) {                        \
        vsh_add(&cmd);                                                  \
    }

#define __VSH_CMD_DEF(name, desc, cmd, tag)                             \
    static vsh_cmd_t V_CONCAT(cmd, tag) = {                             \
        name, desc, cmd                                                 \
    };                                                                  \
    __VSH_CMD_DEF__(V_CONCAT(cmd, tag), tag)

#define VSH_CMD_DEF(name, desc, cmd)                                    \
    __VSH_CMD_DEF(name, desc, cmd, __LINE__)

/* typedefs */

typedef struct vsh_cmd {
    const char     * name;
    const char     * desc;
    int           (* cmd)(vsh_t *sh, int argc, char *argv[]);
    vslist_t         node;
} vsh_cmd_t;

void vsh_add(vsh_cmd_t *cmd);

#else

#define VSH_SECTION     shell

#define __VSH_CMD_DEF(name, desc, cmd, tag)                             \
    const vsh_cmd_t V_CONCAT(cmd, tag) v_section(VSH_SECTION) = {       \
        name, desc, cmd                                                 \
    }

#define VSH_CMD_DEF(name, desc, cmd)                                    \
    __VSH_CMD_DEF(name, desc, cmd, __LINE__)

/* typedefs */

typedef struct vsh_cmd {
    const char     * name;
    const char     * desc;
    int           (* cmd)(vsh_t *sh, int argc, char *argv[]);
} vsh_cmd_t;

#endif // CONFIG_CONSTRUCTOR

/* externs */

int vsh_loop(uintptr_t arg);
int vsh_printf(vsh_t *sh, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSH_H__ */
