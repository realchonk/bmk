#ifndef HAVE_REALLOCARRAY
extern void *reallocarray ();
#endif /* HAVE_REALLOCARRAY */

#ifdef HAVE_ERR_H
# include <err.h>
#else
extern void errx ();
extern void err ();
extern void warnx ();
extern void warn ();
#endif /* HAVE_ERR_H */

#ifdef HAVE_FNMATCH_H
# include <fnmatch.h>
#else
extern fnmatch ();
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#else
extern char *basename ();
extern char *dirname ();
#endif /* HAVE_LIBGEN_H */

#ifndef HAVE_STRDUP
extern char *strdup ();
#endif
