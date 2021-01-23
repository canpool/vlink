/**
 * Copyright (c) [2019] Huawei Technologies Co.,Ltd
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

/*
high level description
----------------------

0) vsh struct

   0.0 vsh is assumed to support more than one entity, more than way:

       > it can be openned in two uart consoles at the same time
       > it can be openned through eth and uart at the same time

       for each entity, these is a <vsh_t> struct for it

   0.1 vsh support customer added command, as this kernel is designed as a
       real-time embedded system, it only support register command statically,
       meaning at build time

       commands are descripted by the struct of <vsh_cmd_t>

1) cli editing

the customer input is held in a buffer in vsh_t::input_buff, and this buffer
is divided into three parts, they are refered by:

    * vsh_t::input_idx
    * vsh_t::compl_idx
    * vsh_t::shift_idx

all of them begin from zero and the sum of them is not greater than the size of
the buffer subtract one for a ending '\0'. and so there are three Implicit buffers:

    * input buffer, it is just vsh_t::input_buff
    * compl buffer, it is just vsh_t::input_buff + vsh_t::input_idx it is
                    used when <tab> or <ctrl-i> keys hit and process the command
                    completion
    * shift buffer, it is special, it is used to save the characters when <left>,
                    <home> or <ctrl-b> keys hit, its base address is the end of
                    vsh_t::input_buff and increase from high to low just like
                    stacks

these three buff/index shown as:

 .-------------------------- vsh->input_buff -------------------------.
/                                                                      \
+---------------------+---------------------+-----+---------------------+
|/////////////////////|\\\\\\\\\\\\\\\\\\\\\|     |/////////////////////|
+---------------------+---------------------+-----+---------------------+
|0           input_idx|0           compl_idx|empty|shift_idx           0|
|increase direction ->|increase direction ->|empty|<- increase direction|
+---------------------+---------------------+-----+---------------------+
|characters before the|completion buff, size|     |characters behind the|
|cursor               |is usually 0         |     |cursor               |
                                            ^
                                          cursor

for example, if there is a command with the name of "test-cmd", and then you
just typed in the word "tes foo", and then 4 <left> keys, and then the command
line should be:

    vsh> tes foo
    $$$$$iiissss

> the characters above '$' is the command line prompt
> the characters above 'i' is the "input" area, the first part in the tab
> the characters above 's' is the "shift" area, the last part in the tab
> and the cursor now is just between "s "

And then type the "tab" key, and then the command line should be:

    vsh> test-cmd foo
    $$$$$iiicccccssss

> the characters above 'c' is the "compl" area, the second part in the tab
> and the cursor now is just between "d "

NOTE: even you see the string in command line is *one* continuous string, it
may be three parts in the <input_buff>:

     .-- input_buff (with the content of "rt-")
    /
    +-------------------+
    |test-cmd~~~~~~~oof |
    +-------------------+
        |    \     /   \
        |     `-+-'     `-- shift_buff growth down (with the content of "oof ")
        \       '-- scratch, not used (hold garbage content)
         `-- compl_buff (with the content of "t-cmd")

when processing the "tab", when any other key except "tab" hit, the current
command will be selected, the "compl" area will be appended to the "input"
area, the "compl" area will be 0 again

before processing the command, these three area will be join to one continuous
string.

2) history processing

the vsh support command history, but the number of history commands is
limited, for saving memory, the buffer hold the history is fixed. so when the
buffer is full, the oldest command is removed. this is just a typical fifo
behavior, so the history commands will a ending NUL are saved in a ring buffer,
implemented in <vring.c>, just like this:

vsh_t::his_cmd:

     .-- the ring buffer start of <vsh_t::his_cmd>
    /
    | arg1 arg2\0cmd0 0\0cmd1 arg0 arg1\0cmd2 arg0 arg1 arg2\0cmd3 arg0|
     333333333333000000001111111111111111222222222222222222222333333333
                 ^       ^               ^                    ^
              idx0(11) idx1(18)        idx2(33)             idx2(54)

as all history commands are saved in the <his_cmd> ring, and every records have
different length, so the index (the ring tail when inserting) must be saved as
a reference so the record can be get back. as the indexes should also be a fifo
behavior, so the indexes are also saved in a ring (vsh_t::his_idx).

so the previous example of <vsh_t::his_cmd> may have following index ring:

     .-- the ring buffer start of <vsh_t::his_idx>
    /
    | 0x000b | 0x0012 | 0x0021 | 0x0036 |
     idx0(11) idx1(18) idx2(33) idx3(54)

in storage, the two rings looks like this:

    +------+------+---+------+
    | idx0 | idx1 | ~ | idxn |                (his_idx ring)
    +------+------+---+------+
    |      \          \
    |       `---+      `--------+
    |           |               |
    +-----------+-----------+---+-----------+
    | cmd0 args | cmd1 args | ~ | cmdn args | (his_cmd ring)
    +-----------+-----------+---+-----------+

when a record in <vsh_t::his_cmd> is overwrote, the corresponding index saved
in <vsh_t::his_idx> must be removed. as the overwrote only happen when the ring
is full:

    +----+
    |    |`\
    |    | |
    |    | +- history ring buff, and is full now
    |    | |
    |    | /
    +----+'
    |    | <- a index for history (refered as his_idx)
    +----+

    so there must be: (ring->tail - his_idx) > (ring->size)

every time a new history is inserted, the overwrote indexes is removed.

3) command registeration and management

commands are managed in a static array

4) vsh front-end

this library is designed for different shells (as descripted in section 0),
different front-end should implement there own support routines and then invoke
this library.
a front-end should implement three function pointers:
    int  (* getchar) (uintptr_t);
    void (* putchar) (uintptr_t, char);
a front-end should define there own vsh_t control struct

to add a new shell front-end please refer to <vsh_uart.c>

5) special keys and shortcuts

ascii code from 0x00 ~ 0x1f are for NUL, ctrl-x, ESC and other invisable keys
ascii code from 0x20 ~ 0x7e are visiable keys
ascii code 0x7f are for del
code from 0x80 ~ 0xff is not really standard and this shell will not support them
functional keys like <home>, <end>, <up>, <down>, <left>, <right> usually generate
more than one key by the terminal, and different terminal have different behavior
so the unsupported codes from 0x80 ~ 0x85 are used to stand for these special
keys, a shell front-end should convert these keys to the codes this library can
recognize (0x80 ~ 0x85), please refer to <vsh_uart.c> as an example.

vsh support some shortcut keys (have similar behovior has them in other
shells):

    * ctrl-a, move cursor to the front of line, just like <home>
    * ctrl-b, move cursor backward, just like <left>
    * ctrl-c, cancel the input, and start a new prompt
    * ctrl-d, kill a character behind the cursor, just like <del>
    * ctrl-e, move cursor to the end of line, just like <end>
    * ctrl-f, move cursor forward, just like <right>
    * ctrl-h, kill a character before the cursor, just like <baskcpase>
    * ctrl-i, command completion, just like <tab>
    * ctlr-l, clean the screen
    * ctrl-j, executive the command have been typed, behavior same as <enter>
    * ctrl-k, kill all characters until to the end of line
    * ctrl-m, executive the command have been typed, behavior same as <enter>
    * ctrl-n, iterate history backward, just like <down>
    * ctrl-o, executive the command have been typed, behavior same as <enter>
    * ctrl-p, iterate history forward, just like <up>
    * ctrl-u, kill all characters until to the begin of line
    * ctrl-w, kill a word before cursor
    * <home>, move cursor to the front of line
    * <end>,  move cursor to the end of line
    * <up>,   iterate history forward
    * <down>, iterate history backward
    * <del>,  kill a character behind the cursor
*/

#include "vsh.h"
#include "vprintf.h"

#include <stdlib.h>
#include <string.h>

// the macros putc and getc in <stdio.h>
#ifdef putc
#undef putc
#endif

#ifdef getc
#undef getc
#endif

/* externs */

/* locals */

static size_t        vsh_max_len = 0;
#ifdef CONFIG_CONSTRUCTOR
static vslist_t      vsh_cmds = VSLIST_INIT(vsh_cmds);
#else
static vsh_cmd_t    *vsh_cmds;
static unsigned int  vsh_nr_cmds;
#endif // CONFIG_CONSTRUCTOR

v_inline bool __is_buff_full(vsh_t *sh)
{
    int len = sh->input_idx + sh->compl_idx + sh->shift_idx;

    /* reserve one byte for the ending '\0' */

    return len == (CONFIG_VSH_BUF_SIZE - 1);
}

v_inline void __input_buff_put(vsh_t *sh, char ch)
{
    if (__is_buff_full(sh)) {
        return;
    }
    sh->input_buff[sh->input_idx++] = ch;
}

v_inline char __input_buff_get(vsh_t *sh, int idx)
{
    return sh->input_buff[idx];
}

v_inline void __compl_buff_put(vsh_t *sh, char ch)
{
    if (__is_buff_full(sh)) {
        return;
    }
    sh->input_buff[sh->input_idx + sh->compl_idx++] = ch;
}

v_inline char __compl_buff_get(vsh_t *sh, int idx)
{
    return sh->input_buff[sh->input_idx + idx];
}

v_inline void __shift_buff_put(vsh_t *sh, char ch)
{
    if (__is_buff_full(sh)) {
        return;
    }
    sh->input_buff[CONFIG_VSH_BUF_SIZE - (++sh->shift_idx)] = ch;
}

v_inline char __shift_buff_get(vsh_t *sh, int idx)
{
    return sh->input_buff[CONFIG_VSH_BUF_SIZE - idx - 1];
}

v_inline void __print_banner(vsh_t *sh)
{
    vsh_printf(sh, "\nWellcome to %s!\n", CONFIG_VSH_WELLCOME);
    vsh_printf(sh, "build time: %s %s\n", __DATE__, __TIME__);
    vsh_printf(sh, "build tool: %s\n",
#if defined(__KEIL__)
                  "ARM KEIL"
#endif
#if defined(__SESA__)
                  "Segger Embedded Studio for ARM"
#endif

#if defined(__clang__)
                  "+LLVM"
#elif defined(__GNUC__)
                  "+GNUC"
#elif defined(_MSC_VER)
                  "+MSVC"
#endif

    );
}

v_inline void __print_prompt(vsh_t *sh)
{
    int i;

    vsh_printf(sh, "\r%s> ", CONFIG_VSH_PROMPT);

    for (i = 0; i < sh->line_end; i++) {
        sh->putc(sh->arg, ' ');
    }
    while (sh->line_end) {
        sh->putc(sh->arg, '\b');
        sh->line_end--;
    }

    sh->position = 0;
}

/**
 * refresh the command line interface show
 */
v_inline void __print_cli(vsh_t *sh)
{
    int i, kill;

    if (sh->input_idx > sh->position) {
        while (sh->input_idx > sh->position) {
            sh->putc(sh->arg, __input_buff_get(sh, sh->position));
            sh->position++;
        }
    } else {
        /* must processing tab, go to the end of input_buff */
        while (sh->position > sh->input_idx) {
            sh->putc(sh->arg, '\b');
            sh->position--;
        }
    }

    for (i = 0; i < sh->compl_idx; i++) {
        sh->putc(sh->arg, __compl_buff_get(sh, i));
    }
    sh->position += sh->compl_idx;

    kill = sh->line_end - (sh->position + sh->shift_idx);
    sh->line_end = sh->position + sh->shift_idx;
    if (kill == 0) {
        return;
    }

    i = sh->shift_idx;
    while (i) {
        sh->putc(sh->arg, __shift_buff_get(sh, --i));
    }

    kill = kill < 0 ? 0 : kill;

    /* kill garbage left by previous command in the end */
    for (i = 0; i < kill; i++) {
        sh->putc(sh->arg, ' ');
    }

    /* move cursor back to the front of "shift" position */
    for (i = 0; i < kill + sh->shift_idx; i++) {
        sh->putc(sh->arg, '\b');
    }
}

static int __tab_loop_pfn(vsh_t *sh, const vsh_cmd_t *cmd)
{
    int         ret;
    const char *name = cmd->name;
    char       *buf  = sh->input_buff;
    size_t      len  = sh->input_idx;

    while ((*buf == ' ') && len) {
        buf++;
        len--;
    }

    if (strncmp(buf, name, len)) {
        return 0;
    }

    if (name[len] == '\0') {
        return 0;
    }

    name += len;

    sh->compl_idx = 0;

    while (*name) {
        __compl_buff_put(sh, *name++);
    }

    __print_cli(sh);

    ret = sh->getc(sh->arg);

    return ret == '\t' ? 0 : ret;
}

v_inline int __cmd_foreach(vsh_t *sh, int (*pfn)(vsh_t *, const vsh_cmd_t *))
{
    int          ret;

#ifdef CONFIG_LINUX
    vsh_cmd_t   *cmd = NULL;

    vslist_foreach_entry(cmd, &vsh_cmds, vsh_cmd_t, node) {
        ret = pfn(sh, cmd);
#else
    unsigned int i;
    for (i = 0; i < vsh_nr_cmds; i++) {
        ret = pfn(sh, &vsh_cmds[i]);
#endif
        /* if pfn return non-zero value means stop the iteration */
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

v_inline int __tab_loop(vsh_t *sh)
{
    return __cmd_foreach(sh, __tab_loop_pfn);
}

v_inline vsh_cmd_t *__find_cmd(char *name)
{
#ifdef CONFIG_LINUX
    vsh_cmd_t   *cmd = NULL;

    vslist_foreach_entry(cmd, &vsh_cmds, vsh_cmd_t, node) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }
#else
    unsigned int i;

    for (i = 0; i < vsh_nr_cmds; i++) {
        if (strcmp(vsh_cmds[i].name, name) == 0) {
            return &vsh_cmds[i];
        }
    }
#endif
    return NULL;
}

v_inline void __cursor_raw_left(vsh_t *sh)
{
    __shift_buff_put(sh, __input_buff_get(sh, --sh->input_idx));
    sh->putc(sh->arg, '\b');
    sh->position--;
}

v_inline void __cursor_left(vsh_t *sh)
{
    if (sh->input_idx != 0) {
        __cursor_raw_left(sh);
    }
}

v_inline void __cursor_home(vsh_t *sh)
{
    while (sh->input_idx != 0) {
        __cursor_raw_left(sh);
    }
}

v_inline void __cursor_raw_right(vsh_t *sh)
{
    __input_buff_put(sh, __shift_buff_get(sh, --sh->shift_idx));
    sh->putc(sh->arg, __input_buff_get(sh, sh->input_idx - 1));
    sh->position++;
}

v_inline void __cursor_right(vsh_t *sh)
{
    if (sh->shift_idx != 0) {
        __cursor_raw_right(sh);
    }
}

v_inline void __cursor_end(vsh_t *sh)
{
    while (sh->shift_idx != 0) {
        __cursor_raw_right(sh);
    }
}

v_inline void __kill_forward(vsh_t *sh)
{
    if (sh->shift_idx != 0) {
        sh->shift_idx--;
    }
}

v_inline void __kill_to_end(vsh_t *sh)
{
    while (sh->shift_idx != 0) {
        sh->shift_idx--;
    }
}

v_inline void __kill_backward(vsh_t *sh)
{
    if (sh->input_idx) {
        sh->input_idx--;
    }
}

v_inline void __kill_to_home(vsh_t *sh)
{
    while (sh->input_idx) {
        sh->input_idx--;
    }
}

v_inline void __kill_word(vsh_t *sh)
{
    while (sh->input_idx != 0) {
        if (__input_buff_get(sh, sh->input_idx - 1) != ' ') {
            break;
        }
        __kill_backward(sh);
    }

    while (sh->input_idx != 0) {
        if (__input_buff_get(sh, sh->input_idx - 1) == ' ') {
            break;
        }
        __kill_backward(sh);
    }
}

v_inline int __str2arg(char *argstr, char *argv[])
{
    int argc = 0;

    while (*argstr) {
        if (*argstr == ' ') {
            *argstr = '\0';
        } else {
            if (argc == CONFIG_VSH_NR_ARGS) {
                break;
            }
            if ((argc == 0) || (argstr[-1] == '\0')) {
                argv[argc++] = argstr;
            }
        }
        argstr++;
    }

    return argc;
}

v_inline uint16_t __get_his_idx(vsh_t *sh, size_t meta_idx)
{
    union {
        uint16_t      u16;
        unsigned char ch[2];
    } u_idx;

    u_idx.ch[0] = vring_peek(&sh->his_idx, (meta_idx << 1) + 0);
    u_idx.ch[1] = vring_peek(&sh->his_idx, (meta_idx << 1) + 1);

    return u_idx.u16;
}

v_inline void __put_his_idx(vsh_t *sh, uint16_t idx)
{
    union {
        uint16_t      u16;
        unsigned char ch[2];
    } u_idx;

    u_idx.u16 = idx;

    vring_putc_force(&sh->his_idx, u_idx.ch[0]);
    vring_putc_force(&sh->his_idx, u_idx.ch[1]);
}

/**
 * check if the command inserting is the same with the last one
 */
v_inline bool __his_is_last(vsh_t *sh, int argc, char *argv[])
{
    int    i = 0;
    size_t idx;

    /* check if the same with the last command */
    idx = __get_his_idx(sh, vring_len(&sh->his_idx) / sizeof(uint16_t) - 1);
    idx = idx - sh->his_cmd.head;

    do {
        char *part = argv[i];
        char  ch;

        do {
            if (*part != vring_peek(&sh->his_cmd, idx++)) {
                return false;
            }
        } while (*++part);

        ch = vring_peek(&sh->his_cmd, idx++);

        switch (ch) {
        case '\0':
            return i == (argc - 1);
        case ' ':
            continue;
        default:
            return false;
        }
    } while (++i != argc);

    return false;
}

v_inline void __insert_his(vsh_t *sh, int argc, char *argv[])
{
    int    i = 0;
    size_t idx;

    /* check if the same with the last command */
    if (__his_is_last(sh, argc, argv)) {
        return;
    }

    /* push the insert point of <his_cmd> ring to <his_idx> ring */
    __put_his_idx(sh, (uint16_t)sh->his_cmd.tail);

    /* put argv [0 ~ n - 1] to <his_cmd> ring with an extra '\0' in the end */
    while (1) {
        vring_put_force(&sh->his_cmd, (unsigned char *)argv[i], strlen(argv[i]));

        if (++i == argc) {
            vring_putc_force(&sh->his_cmd, '\0');
            break;
        }

        vring_putc_force(&sh->his_cmd, ' ');
    }

    /* remove the indexes in <his_idx> ring for those overwrote command records */
    while (1) {
        idx = __get_his_idx(sh, 0);

        if (sh->his_cmd.tail - idx <= sh->his_cmd.size) {
            break;
        }

        vring_get(&sh->his_idx, (unsigned char *)&idx, sizeof(uint16_t));
    }
}

v_inline void __print_his(vsh_t *sh, size_t idx)
{
    char ch;
    int  i;

    while (sh->position) {
        sh->putc(sh->arg, '\b');
        sh->position--;
    }

    while ((ch = vring_peek(&sh->his_cmd, idx++)) != '\0') {
        sh->putc(sh->arg, ch);
        sh->position++;
    }

    for (i = sh->position; i < sh->line_end; i++) {
        sh->putc(sh->arg, ' ');
    }

    while (i-- != sh->position) {
        sh->putc(sh->arg, '\b');
    }

    sh->line_end = sh->position;
}

/**
 * select (copy to vsh_t::input_buff) the history record referenced by the
 * index in <his_cmd> ring
 */
v_inline void __peak_his(vsh_t *sh, size_t idx)
{
    int i;

    sh->shift_idx = 0;
    sh->input_idx = 0;

    for (i = 0; i < sh->position; i++) {
        __input_buff_put(sh, vring_peek(&sh->his_cmd, idx++));
    }

    idx += sh->line_end - sh->position;

    while (i++ < sh->line_end) {
        __shift_buff_put(sh, vring_peek(&sh->his_cmd, --idx));
    }
}

v_inline int __his_loop(vsh_t *sh)
{
    size_t        idxes = vring_len(&sh->his_idx) / sizeof(uint16_t);
    size_t        itr;
    bool          refresh = true;
    size_t        idx     = 0;
    unsigned char ch;

    if (idxes == 0) {
        return '\0';
    }

    itr = idxes - 1;

    /*
     *               itr-.
     *                   |
     * +------+------+---+------+
     * | idx0 | idx1 | ~ | idxn |                (his_idx ring)
     * +------+------+---+------+
     * |      \          \
     * |       `---+      `--------+
     * |           |               |
     * +-----------+-----------+---+-----------+
     * | cmd0 args | cmd1 args | ~ | cmdn args | (his_cmd ring)
     * +-----------+-----------+---+-----------+
     *
     * assume "cmdn" is the last history command record, and <itr> now is just
     * the index of its corresponding index in <his_idx>, itr is index's index
     */

    while (1) {
        if (refresh) {
            idx = __get_his_idx(sh, itr) - sh->his_cmd.head;
            __print_his(sh, idx);
            refresh = false;
        }

        ch = sh->getc(sh->arg);

        switch (ch) {
        case vsh_vk_ctrla:
        case vsh_vk_home:
            while (sh->position != 0) {
                sh->putc(sh->arg, '\b');
                sh->position--;
            }
            break;
        case vsh_vk_ctrlb:
        case vsh_vk_left:
            if (sh->position != 0) {
                sh->putc(sh->arg, '\b');
                sh->position--;
            }
            break;
        case vsh_vk_ctrlf:
        case vsh_vk_right:
            if (sh->position != sh->line_end) {
                sh->putc(sh->arg, vring_peek(&sh->his_cmd, idx + sh->position));
                sh->position++;
            }
            break;
        case vsh_vk_ctrle:
        case vsh_vk_end:
            while (sh->position != sh->line_end) {
                sh->putc(sh->arg, vring_peek(&sh->his_cmd, idx + sh->position));
                sh->position++;
            }
            break;
        case vsh_vk_ctrlp:
        case vsh_vk_up:
            if (itr != 0) {
                itr--;
                refresh = true;
            }
            break;
        case vsh_vk_ctrln:
        case vsh_vk_down:
            if (++itr == idxes) { /* back to the input buffer */
                __print_prompt(sh);
                __print_cli(sh);
                return 0;
            }
            refresh = true;
            break;
        default:
            __peak_his(sh, idx);
            return ch;
        }
    }
}

static int __cmd_history(vsh_t *sh, int argc, char *argv[])
{
    size_t idxes = vring_len(&sh->his_idx) / sizeof(uint16_t);
    size_t itr;

    for (itr = 0; itr < idxes; itr++) {
        size_t        idx;
        unsigned char ch;

        idx = __get_his_idx(sh, itr) - sh->his_cmd.head;

        do {
            ch = vring_peek(&sh->his_cmd, idx++);
            sh->putc(sh->arg, ch);
        } while (ch != '\0');

        vsh_printf(sh, "\n");
    }

    return 0;
}

VSH_CMD_DEF("history",
            "show command history, continuous same commands will only save once",
            __cmd_history);

static int __show_cmd_pfn(vsh_t *sh, const vsh_cmd_t *cmd)
{
    vsh_printf(sh, "%-*s - %s\n", vsh_max_len, cmd->name, cmd->desc);
    return 0;
}

static int __cmd_help(vsh_t *sh, int argc, char *argv[])
{
    __cmd_foreach(sh, __show_cmd_pfn);
    return 0;
}

VSH_CMD_DEF("?",    "show help content (which you are seeing)", __cmd_help);
VSH_CMD_DEF("help", "show help content (which you are seeing)", __cmd_help);

v_inline int __do_cmd_arg(vsh_t *sh, int argc, char *argv[])
{
    const vsh_cmd_t *cmd = __find_cmd(argv[0]);

    vsh_printf(sh, "\n");

    if (cmd != NULL) {
        return cmd->cmd(sh, argc, argv);
    }

    vsh_printf(sh, "Unknown command \"%s\" - try 'help'\n", argv[0]);

    return -1;
}

v_inline int __do_cmd(vsh_t *sh)
{
    int   argc;
    char *argv[CONFIG_VSH_NR_ARGS];
    int   ret = -1;

    while (sh->shift_idx) {
        __input_buff_put(sh, __shift_buff_get(sh, --sh->shift_idx));
    }

    /* make sure there is an ending NUL */
    sh->input_buff[sh->input_idx] = '\0';

    argc = __str2arg(sh->input_buff, argv);

    if (argc != 0) {
        __insert_his(sh, argc, argv);
        ret = __do_cmd_arg(sh, argc, argv);
    } else {
        sh->putc(sh->arg, '\n');
    }

    return ret;
}

v_inline void __join_compl(vsh_t *sh)
{
    sh->input_idx += sh->compl_idx;
    sh->compl_idx = 0;
}

v_inline int __do_tab(vsh_t *sh)
{
    int ch = __tab_loop(sh);

    if (ch == '\0') {
        /*
         * done a tab loop circle and then another tab here, just
         * reset the compl buffer
         */
        sh->compl_idx = 0;
        __print_cli(sh);
    } else {
        /*
         * any other key, means current completion is selected, just append to
         * the input buffer
         */
        __join_compl(sh);
    }

    return ch;
}

int vsh_printf(vsh_t *sh, const char *format, ...)
{
    va_list args;
    int     nbytes;

    va_start(args, format);
    nbytes = xprintf(format, args, sh->putc, (uintptr_t)sh->arg);
    va_end(args);

    return nbytes;
}

v_inline int __vsh_exec(vsh_t *sh)
{
    while (1) {
        int ch = sh->getc(sh->arg);

    reswitch:

        switch (ch) {
        case vsh_vk_nul:
            continue;
        case vsh_vk_ctrla:
            __cursor_home(sh);
            continue;
        case vsh_vk_ctrlb:
            __cursor_left(sh);
            continue;
        case vsh_vk_ctrlc:
            vsh_printf(sh, "^C\n");
            return -1;
        case vsh_vk_ctrld:
            __kill_forward(sh);
            break;
        case vsh_vk_ctrle:
            __cursor_end(sh);
            continue;
        case vsh_vk_ctrlf:
            __cursor_right(sh);
            continue;
        case vsh_vk_ctrlg:
            continue;
        case vsh_vk_ctrlh: /* backspace */
            __kill_backward(sh);
            break;
        case vsh_vk_ctrli:
            ch = __do_tab(sh);
            goto reswitch;
        case vsh_vk_ctrlj:
            return __do_cmd(sh);
        case vsh_vk_ctrlk:
            __kill_to_end(sh);
            break;
        case vsh_vk_ctrll:
            sh->putc(sh->arg, (unsigned char)ch);
            __print_prompt(sh);
            __print_cli(sh);
            continue;
        case vsh_vk_ctrlm:
            return __do_cmd(sh);
        case vsh_vk_ctrln:
            continue;
        case vsh_vk_ctrlo:
            return __do_cmd(sh);
        case vsh_vk_ctrlp:
            ch = __his_loop(sh);
            goto reswitch;
        case vsh_vk_ctrlq:
        case vsh_vk_ctrlr:
        case vsh_vk_ctrls:
        case vsh_vk_ctrlt:
            continue;
        case vsh_vk_ctrlu:
            __kill_to_home(sh);
            break;
        case vsh_vk_ctrlv:
            continue;
        case vsh_vk_ctrlw:
            __kill_word(sh);
            break;
        case vsh_vk_ctrlx:
        case vsh_vk_ctrly:
        case vsh_vk_ctrlz:
        case vsh_vk_esc:
        case vsh_vk_fs:
        case vsh_vk_gs:
        case vsh_vk_rs:
        case vsh_vk_us:
            continue;
        case vsh_vk_del:
            __kill_forward(sh);
            break;
        case vsh_vk_home:
            __cursor_home(sh);
            continue;
        case vsh_vk_end:
            __cursor_end(sh);
            continue;
        case vsh_vk_up:
            ch = __his_loop(sh);
            goto reswitch;
        case vsh_vk_down:
            continue; /* do nothing */
        case vsh_vk_left:
            __cursor_left(sh);
            continue;
        case vsh_vk_right:
            __cursor_right(sh);
            continue;
        default:
            if (ch & 0x80) {
                continue;
            }
            __input_buff_put(sh, (char)ch);
            break;
        }
        __print_cli(sh);
    }
}

#ifdef CONFIG_CONSTRUCTOR

void vsh_add(vsh_cmd_t *cmd)
{
    if (cmd != NULL) {
        vslist_add_tail(&vsh_cmds, &cmd->node);
    }
}

v_inline void __vsh_init(void)
{
    size_t       len;
    vsh_cmd_t   *cmd = NULL;

    vslist_foreach_entry(cmd, &vsh_cmds, vsh_cmd_t, node) {
        len = strlen(cmd->name);
        if (len > vsh_max_len) {
            vsh_max_len = len;
        }
    }
}

#else

extern char v_section_start (VSH_SECTION)[];
extern char v_section_end   (VSH_SECTION)[];

v_inline void __vsh_init(void)
{
    unsigned int i;
    size_t       len;

    vsh_cmds    = (vsh_cmd_t *) (v_section_start(VSH_SECTION));
    vsh_nr_cmds = (unsigned int)(v_section_end  (VSH_SECTION) -
                                 v_section_start(VSH_SECTION));

    vsh_nr_cmds /= sizeof(vsh_cmd_t);

    for (i = 0; i < vsh_nr_cmds; i++) {
        len = strlen(vsh_cmds[i].name);
        if (len > vsh_max_len) {
            vsh_max_len = len;
        }
    }
}

#endif // CONFIG_CONSTRUCTOR

v_noreturn int vsh_loop(uintptr_t arg)
{
    vsh_t *sh = (vsh_t *)arg;

    __vsh_init();
    __print_banner(sh);

    for (;;) {
        sh->input_idx = 0;
        sh->compl_idx = 0;
        sh->shift_idx = 0;
        sh->line_end  = 0;
        sh->position  = 0;

        __print_prompt(sh);
        __vsh_exec(sh);
    }
}
