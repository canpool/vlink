/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "vfs_jffs2.h"
#include "vfs.h"

#include "jffs2_types.h"
#include <jffs2_kernel.h>
#include <jffs2_page.h>
#include <jffs2_crc32.h>
#include "jffs2_nodelist.h"
#include "jffs2_compr.h"
#include "jffs2_fileio.h"


/* the following should be same with os_sys_stat.h */
#define JFFS2_S_IFMT	 S_IFMT
#define JFFS2_S_IFDIR	 S_IFDIR
#define JFFS2_S_IFREG	 S_IFREG

extern cyg_fileops jffs2_fileops;
extern cyg_fileops jffs2_dirops;
extern cyg_fsops jffs2_fsops;

#define jffs2_open          jffs2_fsops.open
#define jffs2_file_unlink   jffs2_fsops.unlink
#define jffs2_mkdir         jffs2_fsops.mkdir
#define jffs2_rmdir         jffs2_fsops.rmdir
#define jffs2_rename        jffs2_fsops.rename
#define jffs2_link          jffs2_fsops.link
#define jffs2_opendir       jffs2_fsops.opendir
#define jffs2_chdir         jffs2_fsops.chdir
#define jffs2_ops_stat      jffs2_fsops.stat
#define jffs2_getinfo       jffs2_fsops.getinfo
#define jffs2_setinfo       jffs2_fsops.setinfo

#define jffs2_file_read     jffs2_fileops.fo_read
#define jffs2_file_write    jffs2_fileops.fo_write
#define jffs2_file_lseek    jffs2_fileops.fo_lseek
#define jffs2_file_ioctl    jffs2_fileops.fo_ioctl
#define jffs2_file_select   jffs2_fileops.fo_select
#define jffs2_file_fsync    jffs2_fileops.fo_fsync
#define jffs2_file_close    jffs2_fileops.fo_close
#define jffs2_file_fstat    jffs2_fileops.fo_fstat
#define jffs2_file_getinfo  jffs2_fileops.fo_getinfo
#define jffs2_file_setinfo  jffs2_fileops.fo_setinfo

#define jffs2_dir_read      jffs2_dirops.fo_read
#define jffs2_dir_write     jffs2_dirops.fo_write
#define jffs2_dir_lseek     jffs2_dirops.fo_lseek
#define jffs2_dir_ioctl     jffs2_dirops.fo_ioctl
#define jffs2_dir_select    jffs2_dirops.fo_select
#define jffs2_dir_fsync     jffs2_dirops.fo_fsync
#define jffs2_dir_colse     jffs2_dirops.fo_close
#define jffs2_dir_fstat     jffs2_dirops.fo_fstat
#define jffs2_dir_getinfo   jffs2_dirops.fo_getinfo
#define jffs2_dir_setinfo   jffs2_dirops.fo_setinfo


static int jffs2_op_open(struct file *file, const char *path, int flags, int mode)
{
    struct super_block  *sb = (struct super_block *)file->f_mp->mp_data;
    cyg_file            *jffs2_file;
    int                  ret;

    if (sb == NULL) return -1;

    jffs2_file = (cyg_file *)malloc(sizeof(cyg_file));
    if (jffs2_file == NULL) {
        vlog_error("fail to malloc memory in JFFS2, <malloc.c> is needed,"
                "make sure it is added");
        return -1;
    }

    ret = jffs2_open(sb, 0, path, flags, jffs2_file);
    if (ret) {
        vlog_error("jffs2_open() failed, ret=%d", ret);
        free(jffs2_file);
        return -1;
    }

    file->f_data = (void *)jffs2_file;

    if (flags & O_APPEND) {
        off_t ofs = 0;
        jffs2_file_lseek(jffs2_file, &ofs, SEEK_END);
        file->f_offset = ofs;
    }

    return 0;
}

static int jffs2_op_close(struct file *file)
{
    int          ret;
    cyg_file    *jffs2_file = (cyg_file *)file->f_data;

    if (jffs2_file == NULL) return -1;

    ret = jffs2_file_close(jffs2_file);
    if (ret) {
        vlog_error("jffs2_file_close() failed, ret=%d", ret);
        return -1;
    }
    /* release memory */
    free(jffs2_file);
    file->f_data = NULL;

    return 0;
}

static ssize_t jffs2_op_read(struct file *file, char *buff, size_t bytes)
{
    cyg_file   *jffs2_file;
    cyg_uio     uio_s;
    cyg_iovec   iovec;
    int         char_read;
    int         ret;

    jffs2_file = (cyg_file *)(file->f_data);
    if (jffs2_file == NULL) return -1;
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = buff;
    uio_s.uio_iov->iov_len = bytes;
    uio_s.uio_iovcnt = 1; //must be 1
    //uio_s.uio_offset //not used...
    uio_s.uio_resid = uio_s.uio_iov->iov_len; //seem no use in jffs2;

    char_read = jffs2_file->f_offset; /* the current offset */
    ret = jffs2_file_read(jffs2_file, &uio_s);
    if (ret) {
        vlog_error("jffs2_file_read() failed, ret=%d", ret);
        return -1;
    }
    /* update position */
    file->f_offset = jffs2_file->f_offset;
    char_read = jffs2_file->f_offset - char_read;
    return char_read;
}

static ssize_t jffs2_op_write(struct file *file, const char *buff, size_t bytes)
{
    cyg_file   *jffs2_file;
    cyg_uio     uio_s;
    cyg_iovec   iovec;
    int         char_write;
    int         ret;

    jffs2_file = (cyg_file *)(file->f_data);
    if (jffs2_file == NULL) return -1;
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = (void *)buff;
    uio_s.uio_iov->iov_len = bytes;
    uio_s.uio_iovcnt = 1; //must be 1
    //uio_s.uio_offset //not used...
    uio_s.uio_resid = uio_s.uio_iov->iov_len; //seem no use in jffs2;

    char_write = jffs2_file->f_offset;
    ret = jffs2_file_write(jffs2_file, &uio_s);
    if (ret) {
        vlog_error("jffs2_file_write() failed, ret=%d", ret);
        return -1;
    }
    /* update position */
    file->f_offset = jffs2_file->f_offset;
    char_write = jffs2_file->f_offset - char_write;
    return char_write;
}

static off_t jffs2_op_lseek(struct file *file, off_t off, int whence)
{
    cyg_file   *jffs2_file;
    int         ret;

    jffs2_file = (cyg_file *)(file->f_data);
    if (jffs2_file == NULL) return -1;

    /* set offset as current offset */
    ret = jffs2_file_lseek(jffs2_file, &off, whence); // SEEK_SET
    if (ret) {
        vlog_error("jffs2_file_lseek() failed, ret=%d", ret);
        return -1;
    }
    /* update file position */
    file->f_offset = off;
    return off;
}

static int jffs2_op_stat(struct mnt *mp, const char *path, struct stat *stat)
{
    struct super_block *sb = (struct super_block *)mp->mp_data;
    cyg_stat            s = {0};
    int                 ret = 0;

    if (sb == NULL) return -1;

    if (path[0] == '/')
        path++;

    ret = jffs2_ops_stat(sb, sb->s_root, path, &s);
    if (ret) {
        vlog_error("jffs2_ops_stat() failed, ret=%d", ret);
        return -1;
    }

    stat->st_dev = s.st_dev;
    stat->st_ino = s.st_ino;
    stat->st_mode = s.st_mode;
    stat->st_nlink = s.st_nlink;
    stat->st_uid = s.st_uid;
    stat->st_gid = s.st_gid;
    stat->st_rdev = 0;
    stat->st_size = s.st_size;
    stat->st_atime = s.st_atime;
    stat->st_mtime = s.st_mtime;
    stat->st_ctime = s.st_ctime;
    stat->st_blksize = 0;
    stat->st_blocks = 0;

    return 0;
}

static int jffs2_op_unlink(struct mnt *mp, const char *path)
{
    struct super_block *sb = (struct super_block *)mp->mp_data;
    int                 ret;
    cyg_stat            s;

    if (sb == NULL) return -1;

    /* deal path */
    if (path[0] == '/')
        path++;

    /* judge file type, dir is to be delete by rmdir, others by unlink */
    ret = jffs2_ops_stat(sb, sb->s_root, path, &s);
    if (ret) {
        vlog_error("jffs2_ops_stat() failed, ret=%d", ret);
        return -1;
    }

    switch(s.st_mode & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFREG:
        ret = jffs2_file_unlink(sb, sb->s_root, path);
        break;
    case JFFS2_S_IFDIR:
        ret = jffs2_rmdir(sb, sb->s_root, path);
        break;
    default:
        /* unknown file type */
        vlog_error("unknown file type");
        return -1;
    }

    return ret ? -1 : 0;
}

static int jffs2_op_rename(struct mnt *mp, const char *path_old, const char *path_new)
{
    struct super_block *sb = (struct super_block *)mp->mp_data;
    int                 ret;

    if (sb == NULL) return -1;

    if (*path_old == '/')
        path_old += 1;

    if (*path_new == '/')
        path_new += 1;

    ret = jffs2_rename(sb, sb->s_root, path_old, sb->s_root, path_new);
    if (ret) {
        vlog_error("jffs2_rename() failed, ret=%d", ret);
        return -1;
    }

    return 0;
}

static int jffs2_op_sync(struct file *file)
{
    return 0;
}

static int jffs2_op_opendir(struct dir *dir, const char *path)
{
    struct super_block *sb = (struct super_block *)dir->d_mp->mp_data;
    cyg_file           *jffs2_file;
    int                 ret;

    if (sb == NULL) return -1;

    jffs2_file = (cyg_file *)malloc(sizeof(cyg_file));
    if (jffs2_file == NULL) {
        vlog_error("fail to malloc memory in JFFS2, <malloc.c> is needed,"
                "make sure it is added");
        return -1;
    }

    ret = jffs2_opendir(sb, sb->s_root, path, jffs2_file);
    if (ret) {
        vlog_error("jffs2_opendir() failed, ret=%d", ret);
        free(jffs2_file);
        return -1;
    }

#ifdef  CONFIG_JFFS2_NO_RELATIVEDIR
    jffs2_file->f_offset = 2;
#endif

    dir->d_data = (void *)jffs2_file;

    return 0;
}

static int jffs2_op_readdir(struct dir *dir, struct dirent *dent)
{
    struct super_block *sb;
    cyg_file           *jffs2_file;
    cyg_uio             uio_s;
    cyg_iovec           iovec;
    cyg_dirent          jffs2_d;
    struct dirent      *d;
#if !defined (CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE)
    cyg_stat            s;
    struct inode       *d_inode = NULL;
#endif
    char                namelen;
    off_t               d_size = 0;
    int                 ret;

    sb = (struct super_block *)(dir->d_mp->mp_data);
    jffs2_file = (cyg_file*)(dir->d_data);
    if (sb == NULL || jffs2_file == NULL)
        return -1;

    //set jffs2_d
    memset(&jffs2_d, 0, sizeof(cyg_dirent));
    //set CYG_UIO_TAG uio_s
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = &jffs2_d;
    uio_s.uio_iov->iov_len = sizeof(cyg_dirent);;
    uio_s.uio_iovcnt = 1; //must be 1
    uio_s.uio_offset = 0;//not used...
    uio_s.uio_resid = uio_s.uio_iov->iov_len; //seem no use in jffs2;

    d = dent;
    ret = jffs2_dir_read(jffs2_file, &uio_s);
    if (ret || jffs2_d.d_name[0] == 0) {
        return -1;
    }
#if defined (CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE)
    switch(jffs2_d.d_type & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFDIR:
		d->d_type = DT_DIR;
		break;
    default:
		d->d_type = DT_REG;
		break;
    }
#else
    d_inode = (struct inode *)jffs2_file->f_data;
    if (d_inode == NULL) return -1;

    ret = jffs2_ops_stat(sb, d_inode, jffs2_d.d_name, &s);
    if (ret) {
        vlog_error("jffs2_ops_stat() failed, ret=%d", ret);
        return -1;
    }
    /* convert to dfs stat structure */
    switch(s.st_mode & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFDIR:
        d->d_type = DT_DIR;
        break;
    default:
        d->d_type = DT_REG;
        break;
    }
    d_size = s.st_size;
#endif

    /* write the rest fields of struct dirent* dirp  */
    namelen = min(strlen(jffs2_d.d_name) + 1, sizeof(d->d_name)) - 1;
    strncpy(d->d_name, jffs2_d.d_name, namelen);
    d->d_name[namelen] = '\0';
    d->d_size = d_size;

    return 0;
}

static int jffs2_op_closedir(struct dir *dir)
{
    cyg_file   *jffs2_file = (cyg_file *)dir->d_data;
    int         ret;

    if (jffs2_file) return -1;

    ret = jffs2_dir_colse(jffs2_file);
    if (ret) {
        vlog_error("jffs2_dir_colse() failed, ret=%d", ret);
        return -1;
    }
    free(jffs2_file);
    dir->d_data = NULL;

    return 0;
}

static int jffs2_op_mkdir(struct mnt *mp, const char *path, int mode)
{
    struct super_block *sb = (struct super_block *)mp->mp_data;
    int                 ret;

    if (sb == NULL) return -1;

    ret = jffs2_mkdir(sb, sb->s_root, path);
    if (ret) {
        vlog_error("jffs2_mkdir() failed, ret=%d", ret);
        return -1;
    }
    return 0;
}

static int jffs2_op_rmdir(struct mnt *mp, const char *path)
{
    struct super_block *sb = (struct super_block *)mp->mp_data;
    int                 ret;

    if (sb == NULL) return -1;

    ret = jffs2_rmdir(sb, sb->s_root, path);
    if (ret) {
        vlog_error("jffs2_rmdir() failed, ret=%d", ret);
        return -1;
    }
    return 0;
}


static struct file_ops jffs2_ops =
{
    jffs2_op_open,
    jffs2_op_close,
    jffs2_op_read,
    jffs2_op_write,
    jffs2_op_lseek,
    jffs2_op_stat,
    jffs2_op_unlink,
    jffs2_op_rename,
    NULL,               /* ioctl not supported for now */
    jffs2_op_sync,
    jffs2_op_opendir,
    jffs2_op_readdir,
    jffs2_op_closedir,
    jffs2_op_mkdir,
    jffs2_op_rmdir
};

static struct file_system jffs2_fs =
{
    "jffs2",
    &jffs2_ops,
    NULL,
    0
};

int jffs2_init(void)
{
    static int jffs2_inited = 0;

    if (jffs2_inited) {
        return 0;
    }

    if (vfs_init() != 0) {
        return -1;
    }

    if (vfs_register(&jffs2_fs) != 0) {
        vlog_error("failed to register fs!");
        return -1;
    }

    jffs2_inited = 1;

    vlog_info("register jffs2 done!");

    return 0;
}

static struct super_block *jffs2_sb_ptr = NULL;

int jffs2_mount(const char *path, struct mtd_info *mtd)
{
    struct super_block *jffs2_sb = NULL;
    int                 ret = -1;

    if(path == NULL || mtd == NULL)
        return ret;

    jffs2_sb = (struct super_block *)malloc(sizeof(struct super_block));
    if (jffs2_sb == NULL) {
        vlog_error("fail to malloc memory in JFFS2, <malloc.c> is needed,"
                "make sure it is added");
        goto err;
    }
    memset(jffs2_sb, 0, sizeof(struct super_block));

    ret = jffs2_do_mount(jffs2_sb, mtd, 0);
    if (ret) {
        vlog_error("failed to mount jffs2, ret=%d!", ret);
        goto err_free;
    }

    ret = vfs_mount("jffs2", path, jffs2_sb);

    if (ret == 0) {
        jffs2_sb_ptr = jffs2_sb;
        return 0;
    }

    vlog_error("failed to mount!");

err_free:
    if (jffs2_sb != NULL)
        free(jffs2_sb);
err:
    return ret;
}

int jffs2_unmount(const char *path)
{
    if (jffs2_sb_ptr) {
        jffs2_do_umount(jffs2_sb_ptr);
        free(jffs2_sb_ptr);
        jffs2_sb_ptr = NULL;
    }
    return vfs_unmount(path);
}
