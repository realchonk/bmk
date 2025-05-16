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

#ifndef HAVE_REALLOCARRAY
extern void_t *reallocarray ();
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
extern int fnmatch ();
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

#ifndef WORKS_STRSEP
# if HAVE_STRSEP
#  define strsep xstrsep
# endif
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
# if HAVE_UNION_WAIT
#  define WEXITSTATUS(ws) ((ws).w_retcode)
# else
#  define WEXITSTATUS(ws) (((ws) >> 8) & 0xff)
# endif
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

#ifndef HAVE_STDLIB_H
extern void_t *malloc ();
extern void_t *calloc ();
extern void_t *realloc ();
extern char *getenv ();
#endif
