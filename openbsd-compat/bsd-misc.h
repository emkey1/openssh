/*
 * Copyright (c) 1999-2004 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _BSD_MISC_H
#define _BSD_MISC_H

#include "includes.h"

char *ssh_get_progname(char *);
int seed_from_prngd(unsigned char *, size_t);

/* __progname, inside iSH-AOK.
 *
 * A native program here is a C function on a guest task's thread inside the
 * app's ONE address space, not a process, so the host's __progname -- the
 * symbol libSystem fills in at startup -- names the APP. Reading it is why
 * sftp printed "usage: ish [...]" and ssh printed "ish: Could not resolve
 * hostname". Writing it, which every entry point does via
 * `__progname = ssh_get_progname(argv[0])`, is the worse half: that stores
 * into libSystem's global, so a correct value would have renamed the app
 * itself and two concurrent applets would have overwritten each other. Same
 * lesson as the three config.h defines in the previous commit -- being true of
 * the host is exactly WHY it cannot be left to the host.
 *
 * So __progname becomes ours and per task. The accessor hands back the address
 * of a thread-local, which keeps `__progname` an lvalue, so the assignment at
 * each entry point still works untouched.
 *
 * It is a function rather than a plain `extern __thread char *` for the sake
 * of the ~20 `extern char *__progname;` lines in the sources: against a
 * variable they would each need editing (a non-thread-local redeclaration of a
 * thread-local is an error), while against this they expand to
 * `extern char *(*ssh_prognamep());`, a compatible old-style redeclaration of
 * the accessor itself. Nothing else has to change, including in the files this
 * build currently excludes. Same shape, and the same reason, as the kernel's
 * nlibc_optindp() for optind.
 */
char **ssh_prognamep(void);
#undef __progname
#define __progname (*ssh_prognamep())

#ifndef HAVE_SETSID
#define setsid() setpgrp(0, getpid())
#endif /* !HAVE_SETSID */

#ifndef HAVE_SETENV
int setenv(const char *, const char *, int);
#endif /* !HAVE_SETENV */

#ifndef HAVE_SETLOGIN
int setlogin(const char *);
#endif /* !HAVE_SETLOGIN */

#ifndef HAVE_INNETGR
int innetgr(const char *, const char *, const char *, const char *);
#endif /* HAVE_INNETGR */

#if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
int seteuid(uid_t);
#endif /* !defined(HAVE_SETEUID) && defined(HAVE_SETREUID) */

#if !defined(HAVE_SETEGID) && defined(HAVE_SETRESGID)
int setegid(uid_t);
#endif /* !defined(HAVE_SETEGID) && defined(HAVE_SETRESGID) */

#if !defined(HAVE_STRERROR) && defined(HAVE_SYS_ERRLIST) && defined(HAVE_SYS_NERR)
const char *strerror(int);
#endif

#if !defined(HAVE_SETLINEBUF)
#define setlinebuf(a)	(setvbuf((a), NULL, _IOLBF, 0))
#endif

#ifndef HAVE_UTIMES
#ifndef HAVE_STRUCT_TIMEVAL
struct timeval {
	long tv_sec;
	long tv_usec;
}
#endif /* HAVE_STRUCT_TIMEVAL */

int utimes(const char *, struct timeval *);
#endif /* HAVE_UTIMES */

#ifndef HAVE_DIRFD
int dirfd(void *);
#endif

#ifndef AT_FDCWD
# define AT_FDCWD (-2)
#endif

#ifndef HAVE_FCHMODAT
int fchmodat(int, const char *, mode_t, int);
#endif

#ifndef HAVE_FCHOWNAT
int fchownat(int, const char *, uid_t, gid_t, int);
#endif

#ifdef HAVE_FSTATAT
int fstatat(int, const char *, struct stat *, int);
#endif

#ifdef HAVE_UNLINKAT
int unlinkat(int, const char *, int);
#endif

#ifndef HAVE_TRUNCATE
int truncate (const char *, off_t);
#endif /* HAVE_TRUNCATE */

#ifndef HAVE_STRUCT_TIMESPEC
struct timespec {
	time_t	tv_sec;
	long	tv_nsec;
};
#endif /* !HAVE_STRUCT_TIMESPEC */

#if !defined(HAVE_NANOSLEEP) && !defined(HAVE_NSLEEP)
# include <time.h>
int nanosleep(const struct timespec *, struct timespec *);
#endif

#ifndef HAVE_UTIMENSAT
# include <time.h>
/* start with the high bits and work down to minimise risk of overlap */
# ifndef AT_SYMLINK_NOFOLLOW
#  define AT_SYMLINK_NOFOLLOW 0x80000000
# endif
int utimensat(int, const char *, const struct timespec[2], int);
#endif /* !HAVE_UTIMENSAT */

#ifndef HAVE_USLEEP
int usleep(unsigned int useconds);
#endif

#ifndef HAVE_TCGETPGRP
pid_t tcgetpgrp(int);
#endif

#ifndef HAVE_TCSENDBREAK
int tcsendbreak(int, int);
#endif

#ifndef HAVE_UNSETENV
int unsetenv(const char *);
#endif

#ifndef HAVE_ISBLANK
int	isblank(int);
#endif

#ifndef HAVE_GETPGID
pid_t getpgid(pid_t);
#endif

#ifndef HAVE_PSELECT
int pselect(int, fd_set *, fd_set *, fd_set *, const struct timespec *,
    const sigset_t *);
#endif

#ifndef HAVE_ENDGRENT
# define endgrent() do { } while(0)
#endif

#ifndef HAVE_KRB5_GET_ERROR_MESSAGE
# define krb5_get_error_message krb5_get_err_text
#endif

#ifndef HAVE_KRB5_FREE_ERROR_MESSAGE
# define krb5_free_error_message(a,b) do { } while(0)
#endif

#ifndef HAVE_PLEDGE
int pledge(const char *promises, const char *execpromises);
#endif

#ifndef HAVE_UNVEIL
int unveil(const char *, const char *);
#endif

/* bsd-err.h */
#ifndef HAVE_ERR
void err(int, const char *, ...) __attribute__((format(printf, 2, 3)));
#endif
#ifndef HAVE_ERRX
void errx(int, const char *, ...) __attribute__((format(printf, 2, 3)));
#endif
#ifndef HAVE_WARN
void warn(const char *, ...) __attribute__((format(printf, 1, 2)));
#endif

#ifndef HAVE_LLABS
long long llabs(long long);
#endif

#if defined(HAVE_DECL_BZERO) && HAVE_DECL_BZERO == 0
void bzero(void *, size_t);
#endif

#ifndef HAVE_RAISE
int raise(int);
#endif

#ifndef HAVE_GETSID
pid_t getsid(pid_t);
#endif

#ifndef HAVE_FLOCK
# define LOCK_SH		0x01
# define LOCK_EX		0x02
# define LOCK_NB		0x04
# define LOCK_UN		0x08
int flock(int, int);
#endif

#ifdef FFLUSH_NULL_BUG
# define fflush(x)	(_ssh_compat_fflush(x))
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r(const time_t *, struct tm *);
#endif

#ifndef HAVE_CLOCK_GETTIME
typedef int clockid_t;
#ifndef CLOCK_REALTIME
# define CLOCK_REALTIME	0
#endif
int clock_gettime(clockid_t, struct timespec *);
#endif

#ifndef HAVE_REALPATH
#define realpath(x, y)	(sftp_realpath((x), (y)))
#endif

#endif /* _BSD_MISC_H */
