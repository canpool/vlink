/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __STAT_H__
#define __STAT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct stat {
    unsigned long   st_dev;
    unsigned long   st_ino;
    int             st_mode;
    unsigned long   st_nlink;
    unsigned short  st_uid;
    unsigned short  st_gid;
    unsigned long   st_rdev;
    unsigned long   st_size;
    unsigned long   st_atime;
    unsigned long   st_mtime;
    unsigned long   st_ctime;
    unsigned long   st_blksize;
    unsigned long   st_blocks;
};

#define S_BLKSIZE   1024 /* size of a block */

#if defined(__KERNEL__) || !defined(__GLIBC__) || (__GLIBC__ < 2)

#define S_IFMT      0170000    /* type of file */
#define S_IFSOCK    0140000    /* socket */
#define S_IFLNK     0120000    /* symbolic link */
#define S_IFREG     0100000    /* regular */
#define S_IFBLK     0060000    /* block special */
#define S_IFDIR     0040000    /* directory */
#define S_IFCHR     0020000    /* character special */
#define S_IFIFO     0010000    /* fifo */
#define S_ISUID     0004000    /* set user id on execution */
#define S_ISGID     0002000    /* set group id on execution */
#define S_ISVTX     0001000    /* save swapped text even after use */

#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU     00700   /* (S_IRUSR | S_IWUSR | S_IXUSR) */
#define S_IRUSR     00400   /* read permission, owner */
#define S_IWUSR     00200   /* write permission, owner */
#define S_IXUSR     00100   /* execute/search permission, owner */

#define S_IRWXG     00070   /* (S_IRGRP | S_IWGRP | S_IXGRP) */
#define S_IRGRP     00040   /* read permission, group */
#define S_IWGRP     00020   /* write permission, grougroup */
#define S_IXGRP     00010   /* execute/search permission, group */

#define S_IRWXO     00007   /* (S_IROTH | S_IWOTH | S_IXOTH) */
#define S_IROTH     00004   /* read permission, other */
#define S_IWOTH     00002   /* write permission, other */
#define S_IXOTH     00001   /* execute/search permission, other */

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STAT_H__ */
