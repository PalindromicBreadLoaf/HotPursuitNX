/*
 * Copyright © 2026 PalindromicBreadLoaf (palindromicbreadloaf@tuta.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Link-time newlib gap fills required by nxvk.
 */
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <switch.h>

u32 __nx_applet_type = AppletType_Application;
size_t __nx_heap_size = 0;

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
{
    (void)flags;
    randomGet(buf, buflen);
    return (ssize_t)buflen;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    if (alignment < sizeof(void *) || (alignment & (alignment - 1)) != 0)
        return EINVAL;
    void *p = memalign(alignment, size);
    if (!p)
        return ENOMEM;
    *memptr = p;
    return 0;
}

uid_t getuid(void)  { return 0; }
uid_t geteuid(void) { return 0; }
gid_t getgid(void)  { return 0; }
gid_t getegid(void) { return 0; }

static long get_available_processor_count(void)
{
    u64 coreMask = 0;
    if (R_FAILED(svcGetInfo(&coreMask, InfoType_CoreMask,
                            CUR_PROCESS_HANDLE, 0)) || coreMask == 0)
        return -1;
    return __builtin_popcountll(coreMask);
}

long sysconf(int name)
{
    switch (name)
    {
    case _SC_PAGESIZE:         return 4096;
    case _SC_PHYS_PAGES:       return (3ll * 1024 * 1024 * 1024) / 4096;
    case _SC_NPROCESSORS_CONF:
    case _SC_NPROCESSORS_ONLN: return get_available_processor_count();
    default:                   return -1;
    }
}

int regcomp(regex_t *preg, const char *regex, int cflags)
{
    (void)regex;
    (void)cflags;
    if (preg)
        preg->re_nsub = 0;
    return 0;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch,
            regmatch_t pmatch[], int eflags)
{
    (void)preg;
    (void)string;
    (void)nmatch;
    (void)pmatch;
    (void)eflags;
    return REG_NOMATCH;
}

void regfree(regex_t *preg)
{
    (void)preg;
}

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
    (void)how;
    (void)set;
    if (oldset)
        memset(oldset, 0, sizeof(*oldset));
    return 0;
}

int flock(int fd, int operation)
{
    (void)fd;
    (void)operation;
    return 0;
}

int dirfd(DIR *dirp)
{
    (void)dirp;
    errno = ENOTSUP;
    return -1;
}

int fstatat(int fd, const char *path, struct stat *buf, int flag)
{
    (void)fd;
    (void)path;
    (void)buf;
    (void)flag;
    errno = ENOTSUP;
    return -1;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen,
               struct passwd **result)
{
    (void)uid;
    (void)pwd;
    (void)buf;
    (void)buflen;
    if (result)
        *result = NULL;
    return ENOTSUP;
}
