/*
 * Copyright (c) 2012,2023 Damien Miller <djm@mindrot.org>
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

#include "includes.h"

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "misc.h"
#include "servconf.h"
#include "xmalloc.h"
#include "hostfile.h"
#include "auth.h"

extern __thread ServerOptions options;

/*
 * Configuration of enabled authentication methods. Separate from the rest of
 * auth2-*.c because we want to query it during server configuration validity
 * checking in the sshd listener process without pulling all the auth code in
 * too.
 */

/* "none" is allowed only one time and it is cleared by userauth_none() later */
int none_enabled = 1;
struct authmethod_cfg methodcfg_none = {
	"none",
	NULL,
	&none_enabled
};
/*
 * iSH-AOK: the `enabled` pointers below were &options.something upstream.
 * `options` is thread-local now, and the address of a thread-local is not a
 * compile-time constant, so they are NULL here and auth2_method_enabled()
 * looks them up in the calling thread's options instead. sshconnect2.c's
 * authmethods[] fills its pointers in once per thread, but that table is
 * thread-local itself; these structs are shared, and auth2-*.c take their
 * addresses in static initializers of their own, so they cannot be.
 */
struct authmethod_cfg methodcfg_pubkey = {
	"publickey",
	"publickey-hostbound-v00@openssh.com",
	NULL		/* &options.pubkey_authentication */
};
#ifdef GSSAPI
struct authmethod_cfg methodcfg_gssapi = {
	"gssapi-with-mic",
	NULL,
	NULL		/* &options.gss_authentication */
};
#endif
struct authmethod_cfg methodcfg_passwd = {
	"password",
	NULL,
	NULL		/* &options.password_authentication */
};
struct authmethod_cfg methodcfg_kbdint = {
	"keyboard-interactive",
	NULL,
	NULL		/* &options.kbd_interactive_authentication */
};
struct authmethod_cfg methodcfg_hostbased = {
	"hostbased",
	NULL,
	NULL		/* &options.hostbased_authentication */
};

static struct authmethod_cfg *authmethod_cfgs[] = {
	&methodcfg_none,
	&methodcfg_pubkey,
#ifdef GSSAPI
	&methodcfg_gssapi,
#endif
	&methodcfg_passwd,
	&methodcfg_kbdint,
	&methodcfg_hostbased,
	NULL
};

/*
 * Whether a method is enabled in the calling thread's configuration. Ask this
 * rather than reading cfg->enabled, which is NULL for every method whose flag
 * lives in `options` -- see the comment above methodcfg_pubkey.
 */
int
auth2_method_enabled(const struct authmethod_cfg *cfg)
{
	const int *enabled = cfg->enabled;

	if (cfg == &methodcfg_pubkey)
		enabled = &options.pubkey_authentication;
#ifdef GSSAPI
	else if (cfg == &methodcfg_gssapi)
		enabled = &options.gss_authentication;
#endif
	else if (cfg == &methodcfg_passwd)
		enabled = &options.password_authentication;
	else if (cfg == &methodcfg_kbdint)
		enabled = &options.kbd_interactive_authentication;
	else if (cfg == &methodcfg_hostbased)
		enabled = &options.hostbased_authentication;
	return enabled != NULL && *enabled != 0;
}

/*
 * Check a comma-separated list of methods for validity. If need_enable is
 * non-zero, then also require that the methods are enabled.
 * Returns 0 on success or -1 if the methods list is invalid.
 */
int
auth2_methods_valid(const char *_methods, int need_enable)
{
	char *methods, *omethods, *method, *p;
	u_int i, found;
	int ret = -1;
	const struct authmethod_cfg *cfg;

	if (*_methods == '\0') {
		error("empty authentication method list");
		return -1;
	}
	omethods = methods = xstrdup(_methods);
	while ((method = strsep(&methods, ",")) != NULL) {
		for (found = i = 0; !found && authmethod_cfgs[i] != NULL; i++) {
			cfg = authmethod_cfgs[i];
			if ((p = strchr(method, ':')) != NULL)
				*p = '\0';
			if (strcmp(method, cfg->name) != 0)
				continue;
			if (need_enable) {
				if (!auth2_method_enabled(cfg)) {
					error("Disabled method \"%s\" in "
					    "AuthenticationMethods list \"%s\"",
					    method, _methods);
					goto out;
				}
			}
			found = 1;
			break;
		}
		if (!found) {
			error("Unknown authentication method \"%s\" in list",
			    method);
			goto out;
		}
	}
	ret = 0;
 out:
	free(omethods);
	return ret;
}
