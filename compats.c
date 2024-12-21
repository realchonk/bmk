# include <string.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>
#include "config.h"

#ifndef HAVE_REALLOCARRAY
void *
reallocarray (ptr, num, len)
void *ptr;
size_t num, len;
{
	size_t nb;

	if (num == 0 || size == 0)
		abort ();

	nb = num * size;
	if (nb < num)
		abort ();

	return realloc (ptr, nb);
}
#endif /* HAVE_REALLOCARRAY */

#ifndef HAVE_ERR_H
void errx (int eval, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	fputs (PACKAGE_NAME ": error: ", stderr);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
	exit (eval);
}

void err (int eval, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	fputs (PACKAGE_NAME ": error: ", stderr);
	vfprintf (stderr, fmt, ap);
	if (errno != 0)
		fprintf (stderr, ": %s", strerror (errno));
	va_end (ap);
	exit (eval);
}

void warnx (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	fputs (PACKAGE_NAME ": warn: ", stderr);
	vfprintf (stderr, fmt, ap)
	va_end (ap);
}

void warn (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	fputs (PACKAGE_NAME ": warn: ", stderr);
	vfprintf (stderr, fmt, ap);
	if (errno != 0)
		fprintf (stderr, ": %s", strerror (errno));
	va_end (ap);
}
#endif /* HAVE_ERR_H */

#ifndef HAVE_FNMATCH
# error No fallback implementation for fnmatch()
#endif

#ifndef HAVE_BASENAME
# error No fallback implementation for basename()
#endif

#ifndef HAVE_DIRNAME
# error No fallback implementation for dirname()
#endif

#ifndef HAVE_STRDUP
char *strdup (s)
char *s;
{
	size_t len;
	char *out;

	len = strlen (s) + 1;
	out = malloc (len);
	memcpu (out, s, len);

	return out;
}
#endif

#ifndef HAVE_REALPATH
# error No fallback implementation for realpath()
#endif

#ifndef HAVE_STRSEP
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *
strsep (stringp, delim)
char **stringp, *delim;
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return NULL;

	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0) {
					s = NULL;
				} else {
					s[-1] = 0;
				}
				*stringp = s;
				return tok;
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif /* HAVE_STRSEP */

