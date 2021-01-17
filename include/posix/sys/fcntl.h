/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __FCNTL_H__
#define __FCNTL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * FMODE_EXEC is 0x20
 * FMODE_NONOTIFY is 0x4000000
 * These cannot be used by userspace O_* until internal and external open
 * flags are split.
 * -Eric Paris
 */

/*
 * When introducing new O_* bits, please check its uniqueness in fcntl_init().
 */
#define O_ACCMODE       00000003
#define O_RDONLY        00000000
#define O_WRONLY        00000001
#define O_RDWR          00000002
#ifndef O_CREAT
#define O_CREAT         00000100    /* not fcntl */
#endif
#ifndef O_EXCL
#define O_EXCL          00000200    /* not fcntl */
#endif
#ifndef O_NOCTTY
#define O_NOCTTY        00000400    /* not fcntl */
#endif
#ifndef O_TRUNC
#define O_TRUNC         00001000    /* not fcntl */
#endif
#ifndef O_APPEND
#define O_APPEND        00002000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK      00004000
#endif
#ifndef O_DSYNC
#define O_DSYNC         00010000    /* used to be O_SYNC, see below */
#endif
#ifndef FASYNC
#define FASYNC          00020000    /* fcntl, for BSD compatibility */
#endif
#ifndef O_DIRECT
#define O_DIRECT        00040000    /* direct disk access hint */
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE     00100000
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY     00200000    /* must be a directory */
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW      00400000    /* don't follow links */
#endif
#ifndef O_NOATIME
#define O_NOATIME       01000000
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC       02000000    /* set close_on_exec */
#endif

/* Values for the WHENCE argument to lseek.  */
#ifndef SEEK_SET
#define SEEK_SET        0    /* seek relative to beginning of file */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR        1    /* seek relative to current file position */
#endif
#ifndef SEEK_END
#define SEEK_END        2    /* seek relative to end of file */
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FCNTL_H__ */
