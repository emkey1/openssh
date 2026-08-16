/*	$OpenBSD: getopt.h,v 1.2 2008/06/26 05:42:04 ray Exp $	*/
/*	$NetBSD: getopt.h,v 1.4 2000/07/07 10:43:54 ad Exp $	*/

/*-
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron and Thomas Klausner.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(HAVE_GETOPT_H) && defined(HAVE_GETOPT_OPTRESET)
/* This directory is on the include search path ahead of the system's own
 * getopt.h, so a bare #include <getopt.h> anywhere in the tree would
 * otherwise resolve to this compat shim instead of the real header even
 * when the platform provides one (its struct option/getopt_long/
 * getopt_long_only declarations below are `#if 0`'d out precisely because
 * they assume the system header will be the one actually seen). Forward to
 * the next getopt.h on the search path (the system one) -- but ONLY when
 * the system also has BSD's optreset (HAVE_GETOPT_OPTRESET): openbsd-
 * compat/getopt_long.c's own struct option/getopt_long stay compiled in
 * (its guard is `!HAVE_GETOPT || !HAVE_GETOPT_OPTRESET`) on any libc that
 * has getopt_long but lacks optreset -- glibc among them. Forwarding
 * unconditionally on HAVE_GETOPT_H alone pulls the system's conflicting
 * struct option into the same translation unit as getopt_long.c's own,
 * which is a redefinition error, not a redundant-but-harmless one.
 * Deliberately no _GETOPT_H_ guard around this branch: the system header
 * has its own guard, and defining the same macro name here would make its
 * content get skipped as "already included" without ever having been seen. */
#include_next <getopt.h>
#elif !defined(_GETOPT_H_)
#define _GETOPT_H_

#ifndef __THROW
# if defined __cplusplus
#  define __THROW throw()
# else
#  define __THROW
# endif
#endif

/*
 * GNU-like getopt_long() and 4.4BSD getsubopt()/optreset extensions
 */
#define no_argument        0
#define required_argument  1
#define optional_argument  2

/* Not every translation unit that reaches this fallback branch (i.e. that
 * doesn't take the include_next path above) defines HAVE_GETOPT_H in its
 * own config.h -- notably the vendored third-party/openrsync tree, which
 * has an entirely separate config.h that never mentions getopt at all. The
 * GNU-style struct option/getopt_long/getopt_long_only ABI below is stable
 * across every platform this project targets (Linux glibc, macOS, *BSD),
 * so it's safe to always provide it here rather than gating it further --
 * EXCEPT for openbsd-compat/getopt_long.c itself, which reaches this same
 * fallback branch (via includes.h) on any libc with getopt but no BSD
 * optreset (glibc among them) and ALSO defines this exact struct/functions
 * a few lines further down in that same file -- a real double-definition,
 * not a redundant-but-harmless one. It defines the sentinel below before
 * including anything, specifically to skip this copy. */
#ifndef SMALLCLUE_GETOPT_LONG_C_OWN_STRUCT_OPTION
struct option {
	/* name of long option */
	const char *name;
	/*
	 * one of no_argument, required_argument, and optional_argument:
	 * whether option takes an argument
	 */
	int has_arg;
	/* if not NULL, set *flag to val when option found */
	int *flag;
	/* if flag not NULL, value to set *flag to; else return value */
	int val;
};

int	 getopt_long(int, char * const *, const char *,
	    const struct option *, int *);
int	 getopt_long_only(int, char * const *, const char *,
	    const struct option *, int *);
#endif /* !SMALLCLUE_GETOPT_LONG_C_OWN_STRUCT_OPTION */

#ifndef _GETOPT_DEFINED_
#define _GETOPT_DEFINED_
int	 getopt(int, char * const *, const char *) __THROW;
int	 getsubopt(char **, char * const *, char **) __THROW;

extern   char *optarg;                  /* getopt(3) external variables */
extern   int opterr;
extern   int optind;
extern   int optopt;
extern   int optreset;
extern   char *suboptarg;               /* getsubopt(3) external variable */
#endif
 
#endif /* !_GETOPT_H_ */
