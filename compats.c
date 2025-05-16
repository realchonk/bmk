/*
 * Copyright (c) 2025 Benjamin Stürz <benni@stuerz.xyz>
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

#include "config.h"
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#ifndef PATH_MAX
# ifdef _POSIX_PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  define PATH_MAX 256
# endif
#endif

#if __STDC__ || HAVE_VOID_PTR
# define void_t void
#else
# define void_t char
#endif

#if !defined(__STDC__) && !defined(HAVE_VOID_FUNC)
# define void
#endif

#ifndef __dead
# define __dead
#endif

#ifndef HAVE_STDLIB_H
extern void_t *malloc ();
extern void_t *calloc ();
extern void_t *realloc ();
extern char *mktemp ();
#endif

extern int errno;

#ifndef HAVE_STRERROR
char *
strerror (err)
{
# if HAVE_SYS_ERRLIST
	extern char *sys_errlist[];
	return sys_errlist[err];
# else
	return "unknown error";
# endif
}
#endif

#ifndef HAVE_REALLOCARRAY
void_t *
reallocarray (ptr, num, size)
void_t *ptr;
size_t num, size;
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

errx (eval, fmt, a, b, c, d)
const char *fmt;
long a, b, c, d;
{
	fputs ("mk: error: ", stderr);
	fprintf (stderr, fmt, a, b, c, d);
	fputc ('\n', stderr);
	exit (eval);
}

err (eval, fmt, a, b, c, d)
const char *fmt;
long a, b, c, d;
{
	fputs ("mk: error: ", stderr);
	fprintf (stderr, fmt, a, b, c, d);
	if (errno != 0)
		fprintf (stderr, ": %s", strerror (errno));
	fputc ('\n', stderr);
	exit (eval);
}

warnx (fmt, a, b, c, d)
const char *fmt;
long a, b, c, d;
{
	fputs ("mk: warn: ", stderr);
	fprintf (stderr, fmt, a, b, c, d);
	fputc ('\n', stderr);
}

warn (fmt, a, b, c, d)
const char *fmt;
long a, b, c, d;
{
	fputs ("mk: warn: ", stderr);
	fprintf (stderr, fmt, a, b, c, d);
	if (errno != 0)
		fprintf (stderr, ": %s", strerror (errno));
	fputc ('\n', stderr);
}
#endif /* HAVE_ERR_H */

#ifndef HAVE_FNMATCH
/* TODO: provide an actual implementation of fnmatch() */
int
fnmatch (pattern, string, flags)
const char *pattern, *string;
{
	return 0;
}
#endif

#ifndef HAVE_BASENAME
char *
basename (s)
char *s;
{
	char *t;

	if (s == NULL || *s == '\0')
		return ".";

	/* remove any trailing slashes */
	for (t = s + strlen (s); t > s && t[-1] == '/'; --t);

	if (s == t)
		return "/";

	*t = '\0';
	for (; t > s && t[-1] != '/'; --t);
	return t;
}
#endif

#ifndef HAVE_DIRNAME
char *
dirname (s)
char *s;
{
	char *t;

	if (s == NULL || *s == '\0')
		return ".";

	/* remove any trailing slashes */
	for (t = s + strlen (s); t > s && t[-1] == '/'; --t);

	/* remove basename */
	for (; t > s && t[-1] != '/'; --t);

	/* remove any trailing slashes */
	for (; t > s && t[-1] == '/'; --t);

	if (s == t)
		return "/";

	*t = '\0';
	return s;
}
#endif

#ifndef HAVE_STRDUP
char *
strdup (s)
const char *s;
{
	size_t len;
	char *out;

	len = strlen (s) + 1;
	out = (char *)malloc (len);
	memcpy (out, s, len);

	return out;
}
#endif

#ifndef HAVE_REALPATH
/* TODO: unfuck this unholy shit-infested junk */
char *
realpath (path, resolved)
const char *path;
char *resolved;
{
	if (resolved == NULL)
		resolved = (char *)malloc (PATH_MAX);

	if (*path == '/') {
		/* TODO: this is unsafe and stupid! */
		strncpy (resolved, path, PATH_MAX - 1);
		resolved[PATH_MAX - 1] = '\0';
	} else {
		/* TODO: this is even more unsafe and stupid!!! */
#if HAVE_GETCWD
		getcwd (resolved, PATH_MAX);
#else
		getwd (resolved);
#endif
		strcat (resolved, "/");
		strcat (resolved, path);
	}

	return resolved;
}
#endif

#ifndef WORKS_STRSEP
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

#if HAVE_STRSEP
# define strsep xstrsep
#endif

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
char **stringp;
const char *delim;
{
	const char *spanp;
	char *s, *tok;
	int c, sc;

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

#ifndef HAVE_FMEMOPEN
FILE *
fmemopen (buffer, size, mode)
void_t *buffer;
size_t size;
const char *mode;
{
	char *path, template[16];
	FILE *file;

	assert (mode != NULL);
	memcpy (template, "/tmp/tmp.XXXXXX", 15);

	path = mktemp (template);
	if (path == NULL)
		return NULL;

	file = fopen (path, "w");
	if (file == NULL)
		return NULL;

	fwrite (buffer, 1, size, file);
	return freopen (path, mode, file);
}
#endif

#ifndef HAVE_MEMMOVE
void_t *
memmove (dest, src, len)
void_t *dest;
const void_t *src;
size_t len;
{
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;

	if (d < s) {
		for (; len > 0; --len)
			*d++ = *s++;
	} else if (d > s) {
		d += len;
		s += len;
		for (; len > 0; --len)
			*--d = *--s;
	}

	return dest;
}
#endif

#ifndef HAVE_STRTOL
long
strtol (s, endptr, base)
const char *s;
char **endptr;
{
	long x = 0;
	int d, sign = 1;

	if (*s == '-') {
		sign = -1;
		++s;
	}

	if (base == 0) {
		if (strncmp (s, "0x", 2) == 0 || strncmp (s, "0X", 2) == 0) {
			base = 16;
			s += 2;
		} else if (strncmp (s, "0", 1) == 0) {
			base = 8;
			s += 1;
		} else {
			base = 10;
		}
	}
	
	if (base < 2 || base > 36)
		goto end;

	while (*s != '\0') {
		if (isdigit (*s)) {
			d = *s++ - '0';
		} else if (isupper (*s)) {
			d = *s++ - 'A' + 10;
		} else if (islower (*s)) {
			d = *s++ - 'a' + 10;
		} else {
			break;
		}
		printf ("d = %d\n", d);

		if (d >= base)
			break;


		x = x * base + d;
	}

end:
	if (endptr)
		*endptr = s;
	return sign * x;
}
#endif
