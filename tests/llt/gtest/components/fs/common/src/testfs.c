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

#include "testfs.h"
#include "vconfig.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h> // sleep

// for stat
#include <grp.h>
#include <langinfo.h>
#include <locale.h>
#include <pwd.h>
#include <time.h>

void waiting(unsigned int second)
{
    unsigned int i;
    unsigned int value;

    printf("waiting....");

    for (i = 0; i < second; ++i) {
        value = i + 1;
        printf("%ds", value);
        fflush(stdout);

        sleep(1);

        // backspace
        printf("\b\b");
        while (value /= 10) {
            printf("\b");
        }
    }
    printf("\n");
}

char *sperm(int mode)
{
    static char s[11] = {0};

    memset(s, '-', sizeof(s));

    switch (mode & S_IFMT) {
    case S_IFSOCK:
        s[0] = 's';
        break;
    case S_IFLNK:
        s[0] = 'l';
        break;
    case S_IFREG:
        s[0] = '-';
        break;
    case S_IFBLK:
        s[0] = 'b';
        break;
    case S_IFDIR:
        s[0] = 'd';
        break;
    case S_IFCHR:
        s[0] = 'c';
        break;
    case S_IFIFO:
        s[0] = 'f';
        break;
    default:
        break;
    }

    if ((mode & S_IRUSR) == S_IRUSR) {
        s[1] = 'r';
    }
    if ((mode & S_IWUSR) == S_IWUSR) {
        s[2] = 'w';
    }
    if ((mode & S_IXUSR) == S_IXUSR) {
        s[3] = 'x';
    }

    if ((mode & S_IRGRP) == S_IRGRP) {
        s[4] = 'r';
    }
    if ((mode & S_IWGRP) == S_IWGRP) {
        s[5] = 'w';
    }
    if ((mode & S_IXGRP) == S_IXGRP) {
        s[6] = 'x';
    }

    if ((mode & S_IROTH) == S_IROTH) {
        s[7] = 'r';
    }
    if ((mode & S_IWOTH) == S_IWOTH) {
        s[8] = 'w';
    }
    if ((mode & S_IXOTH) == S_IXOTH) {
        s[9] = 'x';
    }

    return s;
}

void print_stat(const char *name, struct stat *st)
{
    struct passwd * pwd;
    struct group  * grp;
    struct tm     * tm;
    char            datestring[256] = {0};

    /* Print out type, permissions, and number of links. */
    printf("%10.10s", sperm(st->st_mode));
    //    printf("%lo ", (unsigned long)st->st_mode);
    printf("%4d", st->st_nlink);

    /* Print out owner's name if it is found using getpwuid(). */
    if ((pwd = getpwuid(st->st_uid)) != NULL) {
        printf(" %-8.8s", pwd->pw_name);
    } else {
        printf(" %-8d", st->st_uid);
    }

    /* Print out group name if it is found using getgrgid(). */
    if ((grp = getgrgid(st->st_gid)) != NULL) {
        printf(" %-8.8s", grp->gr_name);
    } else {
        printf(" %-8d", st->st_gid);
    }

    /* Print size of file. */
    printf(" %9jd", (intmax_t)st->st_size);

    tm = localtime((const time_t *)&st->st_mtime);
    /* Get localized date string. */
    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);

    printf(" %s %s\n", datestring, name);
}

void list_stat(const char *mp_path, const char *d_name)
{
    char        full_path[384] = {0}; // with mount path
    char        sub_path[512]  = {0}; // remove mount path
    struct stat st;
    struct dir *dir = vfs_opendir(d_name);
    if (dir == NULL) {
        return;
    }
    while (1) {
        struct dirent *entry = vfs_readdir(dir);
        if (entry == NULL || entry->d_name[0] == 0) {
            break;
        }
        memset(&st, 0, sizeof(struct stat));
        // add prefix
#ifdef CONFIG_SPIFFS
        snprintf(full_path, sizeof(full_path), "%s/%s", mp_path, entry->d_name);
#else
        snprintf(full_path, sizeof(full_path), "%s/%s", d_name, entry->d_name);
#endif
        if (vfs_stat(full_path, &st) == -1) {
            continue;
        }
        // remove prefix
#ifdef CONFIG_SPIFFS
        snprintf(sub_path, sizeof(sub_path), "%s", entry->d_name);
#else
        snprintf(sub_path, sizeof(sub_path), "%s/%s", d_name + strlen(mp_path) + 1, entry->d_name); // 1 is '/'
#endif
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            print_stat(sub_path, &st);
            list_stat(mp_path, full_path);
        } else {
            print_stat(sub_path, &st);
        }
    }

    vfs_closedir(dir);
}

void print_dir(const char *d_name, int level)
{
    struct dir *dir = vfs_opendir(d_name);
    if (dir == NULL) {
        return;
    }
    const char *line = "|-- ";
    if (level <= 1) {
        printf(".\n");
        line = "--- ";
    }

    while (1) {
        struct dirent *entry = vfs_readdir(dir);
        if (entry == NULL || entry->d_name[0] == 0) {
            break;
        }

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char tmp_path[384] = {0};
            printf("|%*s%s/\n", level * 4, line, entry->d_name);
            snprintf(tmp_path, sizeof(tmp_path), "%s/%s", d_name, entry->d_name);
            print_dir(tmp_path, level + 1);
        } else {
            printf("|%*s%s\n", level * 4, line, entry->d_name);
        }
    }

    vfs_closedir(dir);
}

int create_file(const char *path)
{
    int fd;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fd = vfs_open(path, O_RDWR | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        return -1;
    }

    return vfs_close(fd);
}
