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
