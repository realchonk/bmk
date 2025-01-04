#ifndef __dead
# define __dead
#endif

#ifndef HAVE_REALLOCARRAY
extern void *reallocarray ();
#endif /* HAVE_REALLOCARRAY */

#ifdef HAVE_ERR_H
# include <err.h>
#else
extern __dead void errx ();
extern __dead void err ();
extern __dead void warnx ();
extern __dead void warn ();
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

#ifndef HAVE_STRSEP
extern char *strsep ();
#endif

#ifndef HAVE_TIMESPEC
struct timespec {
	time_t tv_sec;
	long tv_nsec;
};
#endif

#ifndef HAVE_REALPATH
extern char *realpath ();
#endif

#ifndef HAVE_FMEMOPEN
extern FILE *fmemopen ();
#endif

#ifndef HAVE_MEMMOVE
extern void *memmove ();
#endif

#ifndef WIFEXITED
# define WIFEXITED(ws) (((ws) & 0x00ff) == 0x0000)
#endif

#ifndef WEXITSTATUS
# define WEXITSTATUS(ws) (((ws) >> 8) & 0xff)
#endif

#ifndef PATH_MAX
# ifdef _POSIX_PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  define PATH_MAX 256
# endif
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif

#ifndef HAVE_LSTAT
# define lstat(fd, st) (stat ((fd), (st)))
#endif
