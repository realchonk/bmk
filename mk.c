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
#include <sys/stat.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_FTIME && HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif
#if NEED_SYS_FILE_H
# include <sys/file.h>
#endif
#include <assert.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#include <string.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#if HAVE_STDBOOL_H
# include <stdbool.h>
#elif (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L) && !__bool_true_false_are_defined
# if HAVE__BOOL
#  define bool _Bool
# else
#  define bool int
# endif
# define true 1
# define false 0
#endif
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#if NEED_TIME_H
# include <time.h>
#endif
#include "compats.h"
#include "mk.h"

#define newa(n, T) ((T *)calloc ((n), sizeof (T)))
#define new(T) (newa (1, T))
#define renew(ptr, n, T) ((T *)reallocarray ((ptr), (n), sizeof (T)))

#ifndef MAKEFILE
# define MAKEFILE "Mkfile"
#endif

#ifndef SHELL
# define SHELL "sh"
#endif

#if HAVE_UNION_WAIT
typedef union wait mk_wait_t;
#else
typedef int mk_wait_t;
#endif

extern int errno;

static const char *cpath, *objdir = NULL;
static int verbose = 0, cline = 0, conterr = 0;
static const struct timespec time_zero;

#if HAVE_DESIGNATED_DECLARATORS
# define FIELD(name, value) .name = value
#else
# define FIELD(name, value) value
#endif

static struct macro m_shell = {
	FIELD (next, NULL),
	FIELD (enext, NULL),
	FIELD (prepend, NULL),
	FIELD (name, "SHELL"),
	FIELD (value, SHELL),
	FIELD (help, NULL),
	FIELD (lazy, 0),
}, m_make = {
	FIELD (next, &m_shell),
	FIELD (enext, &m_shell),
	FIELD (prepend, NULL),
	FIELD (name, "MAKE"),
	FIELD (value, NULL),
	FIELD (help, NULL),
	FIELD (lazy, 0),
}, m_dmake = {
	FIELD (next, &m_make),
	FIELD (enext, &m_make),
	FIELD (prepend, NULL),
	FIELD (name, ".MAKE"),
	FIELD (value, NULL),
	FIELD (help, NULL),
	FIELD (lazy, 0),
}, m_makeflags = {
	FIELD (next, &m_dmake),
	FIELD (enext, &m_dmake),
	FIELD (prepend, NULL),
	FIELD (name, "MAKEFLAGS"),
	FIELD (value, NULL),
	FIELD (help, NULL),
	FIELD (lazy, 0),
}, m_dmakeflags = {
	FIELD (next, &m_makeflags),
	FIELD (enext, &m_makeflags),
	FIELD (prepend, NULL),
	FIELD (name, ".MAKEFLAGS"),
	FIELD (value, NULL),
	FIELD (help, NULL),
	FIELD (lazy, 0),
};

static struct macro *globals = &m_dmakeflags;

/* STRING BUFFER */

typedef struct string {
	char *ptr;
	size_t len, cap;
} str_t;

static str_t tmpstr;

void
str_new (s)
str_t *s;
{
	s->len = 0;
	s->cap = 10;
	s->ptr = newa (s->cap + 1, char);
}

void
str_reserve (s, n)
str_t *s;
size_t n;
{
	if (s->cap == 0) {
		s->cap = n;
		s->ptr = newa (s->cap + 1, char);
	} else if ((s->len + n) > s->cap) {
		for (s->cap *= 2; (s->len + n) > s->cap; s->cap *= 2);
		s->ptr = renew (s->ptr, s->cap + 1, char);
	}
}

void
str_free (s)
str_t *s;
{
	free (s->ptr);
	memset (s, 0, sizeof (*s));
}

void
str_putc (s, ch)
str_t *s;
{
	str_reserve (s, 1);
	s->ptr[s->len++] = ch;
}

void
str_write (s, t, n)
str_t *s;
const char *t;
size_t n;
{
	str_reserve (s, n);
	memcpy (s->ptr + s->len, t, n);
	s->len += n;
}

void
str_puts (s, t)
str_t *s;
const char *t;
{
	str_write (s, t, strlen (t));
}

int
str_last (s)
const str_t *s;
{
	return s->len > 0 ? s->ptr[s->len - 1] : EOF;
}

int
str_pop (s)
str_t *s;
{
	return s->len > 0 ? s->ptr[--s->len] : EOF;
}

void
str_chomp (s)
str_t *s;
{
	while (str_last (s) == '\n')
		str_pop (s);
}

void
str_trim (s)
str_t *s;
{
	size_t i;

	while (isspace (str_last (s)))
		str_pop (s);

	for (i = 0; i < s->len && isspace (s->ptr[i]); ++i);

	memmove (s->ptr, s->ptr + i, s->len - i);
	s->len -= i;
}

void
str_reset (s)
str_t *s;
{
	if (s->cap == 0) {
		str_new (s);
	} else {
		s->len = 0;
	}
}

const char *
str_get (s)
str_t *s;
{
	s->ptr[s->len] = '\0';
	return s->ptr;
}

char *
str_release (s)
str_t *s;
{
	char *t;
	s->ptr[s->len] = '\0';
	t = renew (s->ptr, s->len + 1, char);
	memset (s, 0, sizeof (*s));
	return t;
}

/* STRING MISC */

char *
xstrcat (s, t)
const char *s, *t;
{
	char *u;
	size_t len_s, len_t;

	len_s = strlen (s);
	len_t = strlen (t);
	u = newa (len_s + len_t + 1, char);
	memcpy (u, s, len_s);
	memcpy (u + len_s, t, len_t + 1);
	return u;
}

char *
ltrim (s)
const char *s;
{
	for (; isspace (*s); ++s);
	return (char *)s;
}

#define skip_ws(s) (*(s) = ltrim (*(s)))

char *
rtrim (s)
char *s;
{
	size_t i, len;

	len = strlen (s);
	for (i = len; i > 0 && isspace (s[i - 1]); --i)
		s[i - 1] = '\0';

	return s;
}

char *
trim (s)
char *s;
{
	return ltrim (rtrim (s));
}

bool
starts_with (s, prefix)
const char *s, *prefix;
{
	size_t len_s, len_p;

	len_s = strlen (s);
	len_p = strlen (prefix);

	if (len_s < len_p)
		return 0;

	return memcmp (s, prefix, len_p) == 0;
}

/* PATH LOGIC */

static const struct path path_null = { FIELD (type, PATH_NULL), FIELD (name, NULL) };
static const struct path path_super = { FIELD (type, PATH_SUPER), FIELD (name, NULL) };
static struct path tmppath = { FIELD (type, PATH_NAME), FIELD (name, NULL) };

/* return the number of path components (excl. PATH_NULL). */
size_t
path_len (p)
const struct path *p;
{
	size_t i;

	for (i = 0; p[i].type != PATH_NULL; ++i);

	return i;
}

struct path *
path_cpy (old, old_len, new_len)
const struct path *old;
size_t old_len, new_len;
{
	struct path *p;

	p = newa (new_len + 1, struct path);
	memcpy (p, old, old_len * sizeof (struct path));
	p[new_len].type = PATH_NULL;

	return p;
}

struct path *
path_cat (old, comp)
const struct path *old, *comp;
{
	struct path *p;
	size_t len;

	len = path_len (old);

	switch (comp->type) {
	case PATH_NULL:
		p = path_cpy (old, len, len);
		break;
	case PATH_SUPER:
		if (len > 0 && old[len - 1].type != PATH_SUPER) {
			--len;
			p = path_cpy (old, len, len);
			break;
		}
		/* fallthrough */
	case PATH_NAME:
		p = path_cpy (old, len, len + 1);
		p[len] = *comp;
		break;
	default:
		abort ();
	}

	return p;
}

void
path_write (s, p)
str_t *s;
const struct path *p;
{
	size_t i;

	if (p[0].type == PATH_NULL) {
		str_putc (s, '.');
		return;
	}

	for (i = 0; p[i].type != PATH_NULL; ++i) {
		switch (p[i].type) {
		case PATH_NULL:
			abort ();
		case PATH_SUPER:
			str_puts (s, "../");
			break;
		case PATH_NAME:
			str_puts (s, p[i].name);
			str_putc (s, '/');
			break;
		}
	}

	str_pop (s);
}

const char *
path_to_str (p)
const struct path *p;
{
	str_reset (&tmpstr);
	path_write (&tmpstr, p);
	return str_get (&tmpstr);
}

char *
path_basename (p)
const struct path *p;
{
	char *s, *t;

	/* TODO: maybe allocate the buffer beforehand */
	s = realpath (path_to_str (p), NULL);
	t = strdup (basename (s));
	free (s);

	return t;
}

const char *
path_cat_str (dir, file)
const struct path *dir;
const char *file;
{
	str_reset (&tmpstr);
	path_write (&tmpstr, dir);
	str_putc (&tmpstr, '/');
	str_puts (&tmpstr, file);
	return str_get (&tmpstr);
}

struct path *
parse_path (s)
char *s;
{
	size_t len = 0, cap = 4;
	struct path *p;
	const char *t;

	p = newa (cap + 1, struct path);

	while ((t = strsep (&s, "/")) != NULL) {
		if (*t == '\0' || strcmp (t, ".") == 0)
			continue;

		if (len == cap) {
			cap *= 2;
			p = renew (p, cap + 1, struct path);
		}

		if (strcmp (t, "..") == 0) {
			p[len].type = PATH_SUPER;
		} else {
			p[len].type = PATH_NAME;
			p[len].name = strdup (t);
		}
		++len;
	}
	p[len].type = PATH_NULL;

	return p;
}

void
sc_path_into (out, sc)
str_t *out;
const struct scope *sc;
{
	if (sc->parent == NULL) {
		str_putc (out, '.');
		return;
	}

	sc_path_into (out, sc->parent);
	str_putc (out, '/');
	str_puts (out, sc->name);
}

const char *
sc_path_str (sc)
const struct scope *sc;
{
	str_reset (&tmpstr);
	sc_path_into (&tmpstr, sc);
	return str_get (&tmpstr);
}

void
write_objdir (out, sc)
str_t *out;
const struct scope *sc;
{
	if (objdir != NULL) {
		str_puts (out, objdir);
		str_putc (out, '/');
		sc_path_into (out, sc);
	} else {
		str_putc (out, '.');
	}
}

/* OTHER MISC */

struct filetime {
	struct timespec t;
	int obj;
};

#if HAVE_STAT_MTIM
# define stat_get_mtime(mt, st) mt = st.st_mtim
#else
# define stat_get_mtime(mt, st) mt.tv_sec = st.st_mtime, mt.tv_nsec = 0
#endif

int
get_mtime (out, sc, dir, name)
struct filetime *out;
const struct scope *sc;
const struct path *dir;
const char *name;
{
	extern const char *path_cat_str ();
	struct stat st;
	const char *path;

	if (verbose >= 2)
		printf ("get_mtime('%s'): ", name);

	path = path_cat_str (dir, name);

	if (lstat (path, &st) == 0) {
		stat_get_mtime (out->t, st);
		out->obj = 0;
		if (verbose >= 2)
			printf ("found\n");
		return 0;
	}

	if (objdir == NULL)
		goto enoent;

	str_reset (&tmpstr);
	write_objdir (&tmpstr, sc);
	str_putc (&tmpstr, '/');
	str_puts (&tmpstr, name);
	path = str_get (&tmpstr);

	if (lstat (path, &st) == 0) {
		stat_get_mtime (out->t, st);
		out->obj = 1;
		if (verbose >= 2)
			printf ("found in obj\n");
		return 0;
	}

enoent:
	out->t = time_zero;
	out->obj = 0;
	if (verbose >= 2)
		printf ("not found\n");
	return -1;

}

struct timespec
now ()
{
	struct timespec t;

#if HAVE_CLOCK_GETTIME
	clock_gettime (CLOCK_REALTIME, &t);
#endif

#if HAVE_GETTIMEOFDAY && !defined(HAVE_CLOCK_GETTIME)
	struct timeval tv;
	gettimeofday (&tv, NULL);
	t.tv_sec = tv.tv_sec;
	t.tv_nsec = tv.tv_sec;
#endif

#if HAVE_FTIME && !defined(HAVE_CLOCK_GETTIME) && !defined(HAVE_GETTIMEOFDAY)
	struct timeb tb;
	ftime (&tb);
	t.tv_sec = tb.time;
	t.tv_nsec = (long)tb.millitm * 1000000;
#endif

#if !defined(HAVE_FTIME) && !defined(HAVE_CLOCK_GETTIME) && !defined(HAVE_GETTIMEOFDAY)
	time_t tx;
	tx = time (NULL);
	t.tv_sec = tx;
	t.tv_nsec = 0;
#endif

	return t;
}

int
tv_cmp (a, b)
const struct timespec *a, *b;
{
	if (a->tv_sec < b->tv_sec) {
		return -1;
	} else if (a->tv_sec > b->tv_sec) {
		return 1;
	} else if (a->tv_nsec < b->tv_nsec) {
		return -1;
	} else if (a->tv_nsec > b->tv_nsec) {
		return 1;
	} else {
		return 0;
	}
}

#if NDEBUG
# define sc_dir(sc) ((sc)->dir)
# define sc_custom(sc) ((sc)->custom)
#else
struct directory *
sc_dir (sc)
struct scope *sc;
{
	assert (sc->type == SC_DIR);
	return sc->inner.dir;
}

struct custom *
sc_custom (sc)
struct scope *sc;
{
	assert (sc->type == SC_CUSTOM);
	return sc->inner.custom;
}
#endif

/* MKDIR */

void
mkdir_p (dir)
const char *dir;
{
	struct stat st;
	char *copy;

	if (verbose >= 2)
		printf ("mkdir('%s');\n", dir);

	if (mkdir (dir, 0755) == 0)
		return;

	if (errno == EEXIST) {
		if (stat (dir, &st) != 0)
			err (1, "mkdir_p('%s'): stat()", dir);

		if ((st.st_mode & S_IFMT) == S_IFDIR)
			return;

		errx (1, "mkdir_p('%s'): Not a directory", dir);
	}

	if (errno != ENOENT)
		err (1, "mkdir_p('%s')", dir);

	copy = strdup (dir);
	mkdir_p (dirname (copy));
	free (copy);

	if (mkdir (dir, 0755) != 0)
		err (1, "mkdir('%s')", dir);

	return;
}

void
sc_mkdir_p (sc)
const struct scope *sc;
{
	if (objdir == NULL)
		return;

	str_reset (&tmpstr);
	str_puts (&tmpstr, objdir);
	str_putc (&tmpstr, '/');
	sc_path_into (&tmpstr, sc);

	mkdir_p (str_get (&tmpstr));
}

/* MACORS MISC */

/* is macro name */
bool
ismname (ch)
int ch;
{
	return isalnum (ch) || ch == '_' || ch == '.';
}

/* SEARCHING */

struct macro *
find_emacro (sc, name)
struct scope *sc;
const char *name;
{
	struct macro *m;

	if (sc == NULL)
		return NULL;

	for (m = sc_dir (sc)->emacros; m != NULL; m = m->enext) {
		if (strcmp (m->name, name) == 0)
			return m;
	}
	
	return find_emacro (sc->parent, name);
}

struct macro *
find_macro (sc, name)
struct scope *sc;
const char *name;
{
	struct macro *m;

	for (m = sc_dir (sc)->macros; m != NULL; m = m->next) {
		if (strcmp (m->name, name) == 0)
			return m;
	}

	m = find_emacro (sc->parent, name);
	if (m != NULL)
		return m;

	for (m = globals; m != NULL; m = m->next) {
		if (strcmp (m->name, name) == 0)
			return m;
	}

	return NULL;
}

struct template *
find_template (sc, name)
struct scope *sc;
const char *name;
{
	struct template *tm;

	for (tm = sc_dir (sc)->templates; tm != NULL; tm = tm->next) {
		if (strcmp (tm->name, name) == 0)
			return tm;
	}

	return sc->parent != NULL ? find_template (sc->parent, name) : NULL;
}

struct file *
find_file (dir, name)
struct directory *dir;
const char *name;
{
	struct file *f;

	for (f = dir->ftail; f != NULL; f = f->prev) {
		if (strcmp (name, f->name) == 0)
			return f;
	}

	return NULL;
}

struct scope *
find_subdir (sc, name)
struct scope *sc;
const char *name;
{
	struct scope *sub;

	for (sub = sc_dir (sc)->subdirs; sub != NULL; sub = sub->next) {
		if (strcmp (sub->name, name) == 0)
			return sub;
	}
	return NULL;
}

/* MACRO EXPANSION */

#define MAX_EXPAND_DEPTH 64
struct expand_ctx {
	char *target;
	struct dep *deps, *infdeps;
	struct dep *dep0;
	int free_target, depth;
};

static struct expand_ctx ctx_null = {
	FIELD (target, NULL),
	FIELD (deps, NULL),
	FIELD (infdeps, NULL),
	FIELD (dep0, NULL),
	FIELD (free_target, 0),
	FIELD (depth, 0),
};

void
ectx_init (ctx, target, dep0, deps, infdeps)
struct expand_ctx *ctx;
char *target;
struct dep *dep0;
struct dep *deps, *infdeps;
{
	ctx->target = target;
	ctx->dep0 = dep0;
	ctx->deps = deps;
	ctx->infdeps = infdeps;
	ctx->free_target = 0;
	ctx->depth = 0;
}

struct expand_ctx *
ectx_null ()
{
	ctx_null.depth = 0;
	return &ctx_null;
}

void
ectx_file (ctx, sc, f)
struct expand_ctx *ctx;
struct scope *sc;
struct file *f;
{
	str_t tmp;

	if (objdir != NULL) {
		str_new (&tmp);
		write_objdir (&tmp, sc);
		str_putc (&tmp, '/');
		str_puts (&tmp, f->name);
		ctx->target = str_release (&tmp);
		ctx->free_target = 1;
	} else {
		ctx->target = f->name;
		ctx->free_target = 0;
	}

	ctx->deps = ctx->dep0 = f->dhead;
	ctx->infdeps = f->inf != NULL ? f->inf->dhead : NULL;
	if (ctx->dep0 == NULL && ctx->infdeps != NULL)
		ctx->dep0 = ctx->infdeps;
	ctx->depth = 0;
}

void
ectx_free (ctx)
struct expand_ctx *ctx;
{
	if (ctx->free_target)
		free (ctx->target);
}

void
replace_into (out, s, old, new_str)
str_t *out;
const char *s, *old, *new_str;
{
	size_t len_s, len_old;

	len_s = strlen (s);
	len_old = strlen (old);

	if (len_s < len_old || memcmp (s + len_s - len_old, old, len_old) != 0) {
		str_puts (out, s);
		return;
	}

	str_write (out, s, len_s - len_old);
	str_puts (out, new_str);
}

void
replace_all_into (out, s, old, new_str)
str_t *out;
char *s;
const char *old, *new_str;
{
	char *t;
	int x = 0;

	while ((t = strsep (&s, " \t")) != NULL) {
		if (*t == '\0')
			continue;

		replace_into (out, t, old, new_str);
		str_putc (out, ' ');
		x = 1;
	}

	if (x)
		str_pop (out);
}

void
pr_export (out, sc, prefix, m, ctx)
str_t *out;
const struct scope *sc;
const struct path *prefix;
const struct macro *m;
struct expand_ctx *ctx;
{
	extern void expand_macro_into ();

	str_puts (out, m->name);
	str_puts (out, "='");
	expand_macro_into (out, sc, prefix, m, m->name, ctx);
	str_putc (out, '\'');
}

void
dep_write (out, sc, dep)
str_t *out;
const struct scope *sc;
const struct dep *dep;
{
	if (dep->obj && objdir != NULL) {
		write_objdir (out, sc);
		str_putc (out, '/');
	}
	path_write (out, dep->path);
}

void
expand_special_into (out, sc, prefix, name, ctx)
str_t *out;
struct scope *sc;
const struct path *prefix;
const char *name;
struct expand_ctx *ctx;
{
	struct scope *sub;
	struct macro *m;
	struct dep *dep;
	int i, j;

	/* TODO: .SUBDIRS, .EXPORTS */
	assert (ctx != NULL);

	if (strcmp (name, ".SUBDIRS") == 0) {
		if (sc->type != SC_DIR) {
		invsc:
			errx (1, "%s: invalid scope type", sc_path_str (sc));
		}

		sub = sc_dir (sc)->subdirs;
		if (sub == NULL)
			return;

		str_puts (out, sub->name);
		for (sub = sub->next; sub != NULL; sub = sub->next) {
			str_putc (out, ' ');
			str_puts (out, sub->name);
		}
	} else if (strcmp (name, ".EXPORTS") == 0) {
		if (sc->type != SC_DIR)
			goto invsc;

		m = sc_dir (sc)->emacros;
		if (m == NULL)
			return;

		pr_export (out, sc, prefix, m, ctx);
		for (m = m->enext; m != NULL; m = m->enext) {
			str_putc (out, ' ');
			pr_export (out, sc, prefix, m, ctx);
		}

	} else if (strcmp (name, ".OBJDIR") == 0) {
		write_objdir (out, sc);
	} else if (strcmp (name, ".TARGET") == 0) {
		if (ctx->target == NULL)
			errx (1, "%s: cannot use $@ or ${.TARGET} here", sc_path_str (sc));
		str_puts (out, ctx->target);
	} else if (strcmp (name, ".IMPSRC") == 0) {
		/*if (ctx->dep0 == NULL)
			errx (1, "%s: cannot use $< or ${.IMPSRC} here", sc_path_str (sc));*/

		if (ctx->dep0 == NULL)
			return;

		dep_write (out, sc, ctx->dep0);
	} else if (strcmp (name, ".ALLSRC") == 0) {
		if (ctx->target == NULL)
			errx (1, "%s: cannot use $^ or ${.ALLSRC} here", sc_path_str (sc));

		for (dep = ctx->deps; dep != NULL; dep = dep->next) {
			str_putc (out, ' ');
			dep_write (out, sc, dep);
		}
		for (dep = ctx->infdeps; dep != NULL; dep = dep->next) {
			str_putc (out, ' ');
			dep_write (out, sc, dep);
		}
	} else if (strcmp (name, ".TOPDIR") == 0) {
		str_putc (out, '.');
		for (sub = sc->parent; sub != NULL; sub = sub->parent)
			str_puts (out, "/..");
	} else if (strcmp (name, ".MAKEFILES") == 0) {
		for (i = 0, sub = sc; sub != NULL; ++i, sub = sub->parent) {
			for (j = 0; j < i; ++j)
				str_puts (out, "../");
			str_puts (out, sub->makefile);
			str_putc (out, ' ');
		}
		str_pop (out);
	}
}

/* ${name}		just the value of macro called `name`
 * ${name:old=new}	replace `old` with `new`, must be the last modifier
 * ${name:U}		replace each word with its upper case equivalent
 * ${name:L}		replace each word with its lower case equivalent
 * ${name:F}		try searching for files in either ${.OBJDIR} or source directory
 * ${name:E}		replace each word with its suffix
 * ${name:R}		replace each word with everything but its suffix
 * ${name:H}		replace each word with its dirname() equvialent
 * ${name:T}		replace each word with its basename() equvialent
 * ${name:m1:m2...}	multiple modifiers can be combined
 * ${name:Mpattern}	select only words that match pattern
 * ${name:Npattern}	opposite of :Mpattern
 * ${name:Jstring}	join words by string
 * TODO:
 * somehow make this function shorter
 */
void
subst2 (out, sc, prefix, s, ctx)
str_t *out;
struct scope *sc;
const struct path *prefix;
const char **s;
struct expand_ctx *ctx;
{
	extern char *expand_macro ();
	extern void expand_macro_into ();
	extern void subst ();
	struct macro *m;
	struct filetime ft;
	const char *orig = *s;
	char *t, *u, *v, *w;
	const char *pattern;
	str_t name, old_str, new_str;
	int first;

	++ctx->depth;
	if (ctx->depth >= MAX_EXPAND_DEPTH)
		errx (1, "%s: reached maximum expansion depth", path_to_str (prefix));

	/* parse macro name */
	str_new (&name);
	while (**s != '\0') {
		if (ismname (**s)) {
			str_putc (&name, **s);
			++*s;
		} else if (**s == '$') {
			++*s;
			subst (&name, sc, prefix, s, ctx);
		} else {
			break;
		}
	}

	m = find_macro (sc, str_get (&name));
	if (**s == '}') {
		++*s;
		expand_macro_into (out, sc, prefix, m, str_get (&name), ctx);
		str_free (&name);
		return;
	}

	v = expand_macro (sc, prefix, m, str_get (&name), ctx);
	str_free (&name);

	str_new (&old_str);

	while (**s == ':') {
		++*s;

		/* parse modifier, TODO: move this into a function */
		for (str_reset (&old_str); **s != '\0' && **s != '}' && **s != ':' && **s != '='; ) {
			switch (**s) {
			case '$':
				++*s;
				subst (&old_str, sc, prefix, s, ctx);
				break;
			case '\\':
				++*s;
				/* fallthrough */
			default:
				str_putc (&old_str, **s);
				++*s;
				break;
			}
		}

		if (**s == '=') {
			++*s;

			/* TODO: move this into a function */
			for (str_new (&new_str); **s != '\0' && **s != '}'; ) {
				switch (**s) {
				case '$':
					++*s;
					subst (&new_str, sc, prefix, s, ctx);
					break;
				case '\\':
					++*s;
					/* fallthrough */
				default:
					str_putc (&new_str, **s);
					++*s;
					break;
				}
			}

			replace_all_into (out, v, str_get (&old_str), str_get (&new_str));
			str_free (&new_str);
			goto ret;
		} else if (strcmp (str_get (&old_str), "U") == 0) {
			str_new (&new_str);

			for (t = v; *t != '\0'; ++t)
				str_putc (&new_str, toupper (*t));

			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "L") == 0) {
			str_new (&new_str);

			for (t = v; *t != '\0'; ++t)
				str_putc (&new_str, tolower (*t));

			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "F") == 0) {
			str_new (&new_str);

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				if (get_mtime (&ft, sc, prefix, w) == 0 && ft.obj) {
					write_objdir (&new_str, sc);
					str_putc (&new_str, '/');
				}
				str_puts (&new_str, w);
				str_putc (&new_str, ' ');
			}
			str_pop (&new_str);

			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "E") == 0) {
			str_new (&new_str);

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				u = strrchr (w, '.');
				if (u == NULL || strchr (u, '/') != NULL)
					continue;

				str_puts (&new_str, u);
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "R") == 0) {
			str_new (&new_str);

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				u = strrchr (w, '.');
				if (u != NULL && strchr (u, '/') == NULL)
					*u = '\0';

				str_puts (&new_str, w);
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "H") == 0) {
			str_new (&new_str);

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				str_puts (&new_str, dirname (w));
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (strcmp (str_get (&old_str), "T") == 0) {
			str_new (&new_str);

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				str_puts (&new_str, basename (w));
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (str_get (&old_str)[0] == 'M') {
			str_new (&new_str);

			pattern = str_get (&old_str) + 1;

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				if (fnmatch (pattern, w, 0) != 0)
					continue;

				str_puts (&new_str, w);
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (str_get (&old_str)[0] == 'N') {
			str_new (&new_str);

			pattern = str_get (&old_str) + 1;

			for (t = v; (w = strsep (&t, " \t")) != NULL; ) {
				if (*w == '\0')
					continue;

				if (fnmatch (pattern, w, 0) == 0)
					continue;

				str_puts (&new_str, w);
				str_putc (&new_str, ' ');
			}

			str_pop (&new_str);
			free (v);
			v = str_release (&new_str);
		} else if (str_get (&old_str)[0] == 'J') {
			str_new (&new_str);
			pattern = str_get (&old_str) + 1;
			first = 1;

			for (t = v; (w = strsep (&t, " \t")) != NULL; first = 0) {
				if (*w == '\0')
					continue;

				if (!first)
					str_puts (&new_str, pattern);
				str_puts (&new_str, w);
			}

			free (v);
			v = str_release (&new_str);
		} else {
			errx (1, "%s: invalid modifier: ':%s' in '${%s'", sc_path_str (sc), str_get (&old_str), orig);
		}
	}

	str_puts (out, v);

ret:
	if (**s != '}')
		goto invalid;
	++*s;
	str_free (&old_str);
	free (v);
	--ctx->depth;
	return;
invalid:
	errx (1, "%s: invalid macro expansion: '${%s', s = '%s'", sc_path_str (sc), orig, *s);
}

void
subst (out, sc, prefix, s, ctx)
str_t *out;
struct scope *sc;
const struct path *prefix;
const char **s;
struct expand_ctx *ctx;
{
	char buf[3];
	const char *t;
	int ch;

	ch = **s;
	++*s;
	switch (ch) {
	case '$':
		str_putc (out, '$');
		break;
	case '.':
		expand_special_into (out, sc, prefix, ".TOPDIR", ctx);
		break;
	case '@':
		expand_special_into (out, sc, prefix, ".TARGET", ctx);
		break;
	case '<':
		expand_special_into (out, sc, prefix, ".IMPSRC", ctx);
		break;
	case '^':
		expand_special_into (out, sc, prefix, ".ALLSRC", ctx);
		break;
	case '*':
		t = ".IMPSRC:T}";
		subst2 (out, sc, prefix, &t, ctx);
		break;
	case '{':
		subst2 (out, sc, prefix, s, ctx);
		break;
	case '(':
		errx (1, "%s: syntax error: $(...) syntax is reserved for future use, please use ${...} instead.", sc_path_str (sc));
	default:
		if (isalpha (ch)) {
			buf[0] = ch;
			buf[1] = '}';
			buf[2] = '\0';
			t = buf;
			subst2 (out, sc, prefix, &t, ctx);
			break;
		}
		errx (1, "%s: syntax error: invalid escape sequence: $%c%s", sc_path_str (sc), ch, *s);
	}
}

void
expand_into (out, sc, prefix, s, ctx)
str_t *out;
struct scope *sc;
const struct path *prefix;
const char *s;
struct expand_ctx *ctx;
{
	while (*s != '\0') {
		if (*s != '$') {
			str_putc (out, *s++);
			continue;
		}
		++s;
		subst (out, sc, prefix, &s, ctx);
	}
}

void
expand_macro_into (out, sc, prefix, m, name, ctx)
str_t *out;
struct scope *sc;
const struct path *prefix;
const struct macro *m;
const char *name;
struct expand_ctx *ctx;
{
	if (m == NULL) {
		expand_special_into (out, sc, prefix, name, ctx);
		return;
	}

	if (m->prepend != NULL) {
		expand_macro_into (out, sc, prefix, m->prepend, m->prepend->name, ctx);
		str_putc (out, ' ');
	}

	if (m->value == NULL)
		return;

	if (m->lazy) {
		expand_into (out, sc, prefix, m->value, ctx);
	} else {
		str_puts (out, m->value);
	}
}

char *
expand_macro (sc, prefix, m, name, ctx)
struct scope *sc;
const struct path *prefix;
const struct macro *m;
const char *name;
struct expand_ctx *ctx;
{
	str_t tmp;

	str_new (&tmp);
	expand_macro_into (&tmp, sc, prefix, m, name, ctx);
	return str_release (&tmp);
}

char *
expand (sc, prefix, s, ctx)
struct scope *sc;
const struct path *prefix;
const char *s;
struct expand_ctx *ctx;
{
	str_t out;

	if (ctx == NULL)
		ctx = ectx_null ();

	str_new (&out);
	expand_into (&out, sc, prefix, s, ctx);
	str_trim (&out);
	return str_release (&out);
}

/* COMMAND EXECUTION */

char *
get_shell (sc, dir, ctx)
struct scope		*sc;
const struct path	*dir;
struct expand_ctx	*ctx;
{
	struct macro	*m;
	char		*v;

	m = find_macro (sc, "SHELL");
	if (m == NULL) {
		warnx ("BUG: ${SHELL} not found");
		m = &m_shell;
	}

	v = expand_macro (sc, dir, m, "SHELL", ctx);

	return v;
}

char *
evalcom (sc, dir, cmd)
struct scope		*sc;
const struct path	*dir;
const char		*cmd;
{
	char *shell, *args[4];
	ssize_t i, n;
	str_t data;
	pid_t pid;
	int pipefd[2];
	char buf[64 + 1];

	shell = get_shell (sc, dir, NULL);

	args[0] = shell;
	args[1] = "-c";
	args[2] = expand (sc, dir, cmd, NULL);
	args[3] = NULL;

	if (pipe (pipefd) != 0)
		err (1, "pipe()");

	pid = fork ();
	if (pid == -1)
		err (1, "fork()");

	if (pid == 0) {
		close (STDOUT_FILENO);
		close (pipefd[0]);
		if (dup (pipefd[1]) != STDOUT_FILENO)
			err (1, "failed to dup");
		close (STDIN_FILENO);
		if (open ("/dev/null", O_RDONLY) != STDIN_FILENO)
			err (1, "failed to open /dev/null");
		close (pipefd[1]);

		if (chdir (path_to_str (dir)) != 0)
			err (1, "failed to chdir");

		execvp (shell, args);
		err (1, "failed to launch shell");
	} else {
		close (pipefd[1]);

		str_new (&data);

		while ((n = read (pipefd[0], buf, sizeof (buf) - 1)) > 0) {
			for (i = 0; i < n; ++i)
				str_putc (&data, buf[i]);
		}
		close (pipefd[0]);
		wait (NULL);
		free (shell);
		free (args[2]);
		str_chomp (&data);
		return str_release (&data);
	}
}

int
runcom (sc, prefix, cmd, ctx, rule)
struct scope *sc;
const struct path *prefix;
const char *cmd, *rule;
struct expand_ctx *ctx;
{
	char *shell, *ecmd, *args[5];
	pid_t pid;
	mk_wait_t ws;
	int i = 0, q = 0, ign = 0;

	if (*cmd == '@') {
		q = 1;
		++cmd;
	} else if (*cmd == '-') {
		ign = 1;
		++cmd;
	} else if (verbose < 0) {
		q = 1;
	}

	shell = get_shell (sc, prefix, ctx);
	ecmd = expand (sc, prefix, cmd, ctx);

	if (!q) {
		printf ("[%s%s%s] $ %s\n",
			prefix[0].type != PATH_NULL ? path_to_str (prefix) : "",
			prefix[0].type != PATH_NULL && rule != NULL ? "/" : "",
			rule != NULL ? rule : "",
			verbose ? ecmd : cmd
		);
	}

	pid = fork ();
	if (pid < 0)
		err (1, "fork()");

	args[i++] = shell;
	args[i++] = ign ? "-c" : "-ec";
	args[i++] = ecmd;
	args[i] = NULL;

	if (pid == 0) {
		close (STDIN_FILENO);
		if (open ("/dev/null", O_RDONLY) != STDIN_FILENO)
			warn ("%d: open('/dev/null')", STDIN_FILENO);

		if (chdir (path_to_str (prefix)) != 0)
			err (126, "chdir()");

		execvp (shell, args);
		err (127, "exec('%s')", ecmd);
	} else {
		free (shell);
		free (ecmd);
		if (wait (&ws) != pid) {
			warn ("wait()");
			return 254;
		}

		if (!WIFEXITED (ws)) {
			warnx ("%d: process didn't exit", (int)pid);
			return 255;
		}

		return ign ? 0 : WEXITSTATUS (ws);
	}
}

/* EXPRESSION PARSER */

bool
is_truthy (s)
const char *s;
{
	char *endp;
	long x;

	if (*s == '\0')
		return 0;

	x = strtol (s, &endp, 0);

	return *endp != '\0' || x != 0;
}

void
e_command (s, cmd, arg)
const char **s;
const char *cmd;
str_t *arg;
{
	const char *orig = *s;

	*s += strlen (cmd);
	skip_ws (s);
	if (**s != '(')
		errx (1, "%s:%d: expected '(' after 'defined': %s", cpath, cline, orig);

	str_new (arg);
	for (++*s; **s != ')'; ++*s)
		str_putc (arg, **s);
	++*s;

	str_trim (arg);
}

void
e_atom (sc, prefix, s, val)
struct scope *sc;
const struct path *prefix;
const char **s;
str_t *val;
{
	str_t arg;
	int x;

	skip_ws (s);

	if (**s == '"') {
		++*s;

		while (**s != '"') {
			if (**s == '$') {
				++*s;
				subst (val, sc, prefix, s, ectx_null ());
			} else {
				str_putc (val, **s);
				++*s;
			}
		}
		++*s;
	} else if (starts_with (*s, "defined")) {
		e_command (s, "defined", &arg);
		x = find_macro (sc, str_get (&arg)) != NULL;
	comm:
		str_putc (val, x ? '1' : '0');
		str_free (&arg);
	} else if (starts_with (*s, "target")) {
		e_command (s, "target", &arg);
		x = find_file (sc_dir (sc), str_get (&arg)) != NULL;
		goto comm;
	} else {
		errx (1, "%s:%d: invalid expression: '%s'", cpath, cline, *s);
	}
}

void
e_unary (sc, prefix, s, val)
struct scope *sc;
const struct path *prefix;
const char **s;
str_t *val;
{
	skip_ws (s);
	if (**s != '!') {
		e_atom (sc, prefix, s, val);
		return;
	}
	++*s;

	e_unary (sc, prefix, s, val);

	if (is_truthy (str_get (val))) {
		str_reset (val);
		str_putc (val, '0');
	} else {
		str_reset (val);
		str_putc (val, '1');
	}
}

enum compare_type {
	COMP_EQ,
	COMP_NE,
	COMP_LT,
	COMP_LE,
	COMP_GT,
	COMP_GE,
};

bool
e_comp (sc, prefix, s)
struct scope *sc;
const struct path *prefix;
const char **s;
{
	str_t left, right;
	const char *sl, *sr;
	char *el, *er;
	long il, ir;
	enum compare_type cmp;
	int x, icmp;

	str_new (&left);
	e_unary (sc, prefix, s, &left);

	skip_ws (s);

	if (starts_with (*s, "==")) {
		cmp = COMP_EQ;
		*s += 2;
	} else if (starts_with (*s, "!=")) {
		cmp = COMP_NE;
		*s += 2;
	} else if (**s == '<') {
		++*s;
		if (**s == '=') {
			cmp = COMP_LE;
			++*s;
		} else {
			cmp = COMP_LT;
			++*s;
		}
	} else if (**s == '>') {
		++*s;
		if (**s == '=') {
			cmp = COMP_GE;
			++*s;
		} else {
			cmp = COMP_GT;
			++*s;
		}
	} else {
		str_trim (&left);
		x = is_truthy (str_get (&left));
		str_free (&left);
		return x;
	}

	skip_ws (s);
	str_new (&right);
	e_unary (sc, prefix, s, &right);

	str_trim (&left);
	str_trim (&right);

	sl = str_get (&left);
	sr = str_get (&right);
	il = strtol (sl, &el, 0);
	ir = strtol (sr, &er, 0);
	icmp = *sl != '\0' && *sr != '\0' && *el == '\0' && *er == '\0';
	x = icmp ? il - ir : strcmp (sl, sr);
	switch (cmp) {
	case COMP_EQ:
		x = x == 0;
		break;
	case COMP_NE:
		x = x != 0;
		break;
	case COMP_LT:
		x = x < 0;
		break;
	case COMP_LE:
		x = x <= 0;
		break;
	case COMP_GT:
		x = x > 0;
		break;
	case COMP_GE:
		x = x >= 0;
		break;
	default:
		abort ();
	}
	
	str_free (&left);
	str_free (&right);
	return x;
}

bool
e_and (sc, prefix, s)
struct scope *sc;
const struct path *prefix;
const char **s;
{
	int x;

	x = e_comp (sc, prefix, s);
	while (skip_ws (s), starts_with (*s, "&&")) {
		*s += 2;
		x &= e_comp (sc, prefix, s);
	}

	return x;
}

bool
e_or (sc, prefix, s)
struct scope *sc;
const struct path *prefix;
const char **s;
{
	int x;

	x = e_and (sc, prefix, s);
	while (skip_ws (s), starts_with (*s, "||")) {
		*s += 2;
		x |= e_and (sc, prefix, s);
	}

	return x;
}

bool
parse_expr (sc, prefix, s)
struct scope *sc;
const struct path *prefix;
const char *s;
{
	return e_or (sc, prefix, &s);
}

/* PARSER */

char *
readline (file, ln)
FILE *file;
int *ln;
{
	str_t line;
	int ch, eof = 1;

	str_new (&line);

	while (1) {
		ch = fgetc (file);
		switch (ch) {
		case EOF:
			goto ret;
		case '\n':
			eof = 0;
			++*ln;
			goto ret;
		case '\\':
			ch = fgetc (file);
			if (ch == '\n') {
				++*ln;
			} else {
				str_putc (&line, '\\');
				str_putc (&line, ch);
			}
			eof = 0;
			break;
		default:
			str_putc (&line, ch);
			eof = 0;
			break;
		}
	}

ret:
	return eof ? NULL : str_release (&line);
}

struct scope *
new_subdir (parent, name)
struct scope *parent;
const char *name;
{
	struct directory *pdir;
	struct scope *sub;
	
	assert (name != NULL);

	pdir = sc_dir (parent);

	sub = new (struct scope);
	sub->next = pdir->subdirs;
	sub->type = SC_DIR;
	sub->name = strdup (name);
	sub->parent = parent;
	sub->makefile = NULL;
	sub->created = 0;
	pdir->subdirs = sub;

	return sub;
}

struct macro *
new_macro (name, value, help, lazy, prepend)
char *name, *value, *help;
struct macro *prepend;
{
	struct macro *m;
	const char *s;

	assert (name != NULL);
	assert (value != NULL);

	/* check name */
	if (*name == '\0')
		errx (1, "new_macro(): macro name is empty");

	for (s = name; *s != '\0'; ++s) {
		if (!ismname (*s))
			errx (1, "invalid macro name: '%s'", name);
	}

	m = new (struct macro);
	m->next = NULL;
	m->enext = NULL;
	m->prepend = prepend;
	m->name = name;
	m->value = value;
	m->help = help;
	m->lazy = lazy;

	return m;
}

struct file *
new_file (name, rule, time, dhead, dtail, help, inf, obj)
char *name, *help;
struct rule *rule;
struct timespec time;
struct dep *dhead, *dtail;
struct inference *inf;
{
	struct file *f;

	assert (name != NULL);

	f = new (struct file);
	f->next = f->prev = NULL;
	f->name = name;
	f->rule = rule;
	f->dhead = dhead;
	f->dtail = dtail;
	f->mtime = time;
	f->help = help;
	f->inf = inf;
	f->obj = obj;
	f->err = 0;

	return f;
}

struct dep *
new_dep (path)
struct path *path;
{
	struct dep *d;

	assert (path != NULL);

	d = new (struct dep);
	d->next = d->prev = NULL;
	d->path = path;

	return d;
}

struct dep *
new_dep_name (name)
char *name;
{
	struct path *p;

	assert (name != NULL);

	p = newa (2, struct path);
	p[0].type = PATH_NAME;
	p[0].name = name;
	p[1].type = PATH_NULL;

	return new_dep (p);
}

void
dir_add_file (dir, f)
struct directory *dir;
struct file *f;
{
	assert (f->next == NULL && f->prev == NULL);

	if (dir->fhead != NULL) {
		f->prev = dir->ftail;
		dir->ftail->next = f;
	} else {
		dir->fhead = f;
	}
	dir->ftail = f;
}

void
file_add_deps (file, dhead, dtail)
struct file *file;
struct dep *dhead, *dtail;
{
	assert (dhead->prev == NULL);
	assert (dtail->next == NULL);

	if (file->dhead != NULL) {
		dhead->prev = file->dtail;
		file->dtail->next = dhead;
	} else {
		file->dhead = dhead;
	}
	file->dtail = dtail;
}

void
file_add_dep (file, dep)
struct file *file;
struct dep *dep;
{
	file_add_deps (file, dep, dep);
}

char *
strip_comment (s)
char *s;
{
	char *t, *o = s;

	while ((t = strchr (s, '#')) != NULL) {
		if (t == o || isspace (t[-1])) {
			*t = '\0';
			break;
		}
		s = t + 1;
	}

	return o;
}

/* .SUBDIRS: cc make sys # comment */
void
parse_subdirs (sc, dir, s)
struct scope *sc;
const struct path *dir;
char *s;
{
	struct scope *sub;
	char *subdir;
	const char *path, *name;

	strip_comment (s);

	while ((subdir = strsep (&s, " \t")) != NULL) {
		if (*subdir == '\0')
			continue;

		name = trim (subdir);
		sub = new_subdir (sc, name);
		sub->type = SC_DIR;
		sub->makefile = MAKEFILE;

		path = path_cat_str (dir, sub->name);
		if (access (path, F_OK) != 0)
			errx (1, "%s:%d: directory not found: %s", cpath, cline, sub->name);
	}
}

/* .FOREIGN: libfoo libbar # comment */
void
parse_foreign (sc, s)
struct scope *sc;
char *s;
{
	struct custom *cs;
	struct scope *sub;
	const char *name;
	char *subdir;

	strip_comment (s);

	while ((subdir = strsep (&s, " \t")) != NULL) {
		if (*subdir == '\0')
			continue;

		name = trim (subdir);
		sub = new_subdir (sc, name);
		sub->type = SC_CUSTOM;
		sub->makefile = NULL;
		cs = new (struct custom);
		cs->test = NULL;
		cs->exec = NULL;
		sub->inner.custom = cs;
	}
}

/* .EXPORTS: CC CFLAGS # comment */
void
parse_exports (sc, s)
struct scope *sc;
char *s;
{
	struct macro *m;
	const char *name;

	strip_comment (s);

	while ((name = strsep (&s, " \t")) != NULL) {
		if (*name == '\0')
			continue;

		/* check if the macro is already exported */
		for (m = sc_dir (sc)->emacros; m != NULL; m = m->enext) {
			if (strcmp (m->name, name) == 0)
				goto cont; /* already exported */
		}

		m = find_macro (sc, name);
		if (m == NULL)
			errx (1, "%s:%d: no such macro: '%s'", cpath, cline, name);

		m->enext = sc_dir (sc)->emacros;
		sc_dir (sc)->emacros = m;

	cont:;
	}
}

bool
try_add_custom (sc, f)
struct scope *sc;
struct file *f;
{
	struct scope *sub;
	char *name;
	size_t len;
	int ch;

	len = strlen (f->name);
	ch = f->name[len - 1];
	if (ch != '?' && ch != '!')
		return false;

	name = strdup (f->name);
	name[len - 1] = '\0';
	sub = find_subdir (sc, name);
	if (sub == NULL)
		errx (1, "%s: not a subdir: %s", sc_path_str (sc), name);

	if (sub->type != SC_CUSTOM)
		errx (1, "%s: not a custom subdir: %s", sc_path_str (sc), name);

	if (ch == '?') {
		sub->inner.custom->test = f;
	} else {
		sub->inner.custom->exec = f;
	}

	free (name);
	return true;
}

/* TODO: impl .SUFFIXES: */
bool
is_inf (s)
char *s;
{
	char *d;
	int x;

	if (*s != '.')
		return 0;
	++s;
	d = strchr (s, '.');

	if (d == NULL) {
		return strlen (s) <= 3;
	}

	*d = '\0';
	x = strlen (s) <= 3 && strlen (d + 1) <= 3;
	*d = '.';
	return x;
}

struct rule *
parse_rule (sc, dir, s, t, help)
struct scope *sc;
const struct path *dir;
char *s, *t, *help;
{
	struct inference *inf;
	struct filetime ft;
	struct rule *r;
	struct file *f;
	struct dep *dep, *dhead, *dtail;
	char *u, *v, *p;
	int flag;

	r = new (struct rule);
	r->code = NULL;
	dhead = dtail = NULL;

	*t = '\0';

	/* parse deps */
	v = u = expand (sc, dir, t + 1, NULL);
	while ((p = strsep (&v, " \t")) != NULL) {
		if (*p == '\0')
			continue;

		dep = new_dep (parse_path (p));
		if (dhead != NULL) {
			dep->prev = dtail;
			dtail->next = dep;
		} else {
			dhead = dep;
		}
		dtail = dep;
	}	
	free (u);

	/* parse targets */
	u = expand (sc, dir, s, NULL);
	flag = 1;
	if (is_inf (u)) {
		p = strchr (u + 1, '.');
		inf = new (struct inference);
		inf->next = sc_dir (sc)->infs;
		inf->rule = r;
		inf->dhead = dhead;
		inf->dtail = dtail;

		if (p != NULL) {
			*p = '\0';
			inf->from = strdup (u);
			*p = '.';
			inf->to = strdup (p); 
		} else {
			inf->from = strdup (u);
			inf->to = "";
		}

		sc_dir (sc)->infs = inf;
	} else {
		v = u;
		while ((p = strsep (&v, " \t")) != NULL) {
			if (*p == '\0')
				continue;
			/* TODO: check name */

			f = find_file (sc_dir (sc), p);
			if (f == NULL) {
				get_mtime (&ft, sc, dir, p);
					
				f = new_file (
					/* name */ strdup (p),
					/* rule */ r,
					/* time */ ft.t,
					/* dhead*/ dhead,
					/* dtail*/ dtail,
					/* help */ help,
					/* inf  */ NULL,
					/* obj  */ ft.obj
				);
				/* TODO: maybe first do try_add_custom()? */
				dir_add_file (sc_dir (sc), f);
				try_add_custom (sc, f);
				continue;
			}

			flag = 0;

			if (f->help == NULL)
				f->help = help;

			if (dhead != NULL)
				file_add_deps (f, dhead, dtail);
		}
	}
	free (u);
	return flag ? r : NULL;
}

void
parse_assign (sc, dir, s, t, help)
struct scope *sc;
const struct path *dir;
char *s, *t, *help;
{
	struct macro *m, *prepend = NULL;
	char *value;
	int lazy;

	if (t[-1] == '!') {
		t[-1] = '\0';
		lazy = 0;
		value = evalcom (sc, dir, trim (t + 1));
	} else if (t[-1] == '?') {
		/* handle both `?=` and `??=` */
		if (s == (t - 1))
			errx (1, "why are you doing this to me Davids?");

		if (t[-2] == '?') {
			t[-2] = '\0';
			value = getenv (trim (s));
			if (value == NULL) {
				m = find_macro (sc, trim (s));
				if (m != NULL)
					value = m->value;
			}
			if (value == NULL)
				value = strdup (trim (t + 1));
		} else {
			t[-1] = '\0';
			m = find_macro (sc, trim (s));
			value = m != NULL ? m->value : strdup (trim (t + 1));
		}
		lazy = 1;
	} else if (t[-1] == ':') {
		/* handle both `:=` and `::=` */
		if (s == (t - 1))
			errx (1, "why are you doing this to me Davids, again?");
		t[t[-2] == ':' ? -2 : -1] = '\0';
		value = expand (sc, dir, trim (t + 1), NULL);
		lazy = 0;
	} else if (t[-1] == '+') {
		t[-1] = '\0';
		value = strdup (trim (t + 1));
		prepend = find_macro (sc, trim (s));
		lazy = 1;
	} else {
		value = strdup (trim (t + 1));
		lazy = 1;
	}

	m = new_macro (
		/* name  */ strdup (trim (s)),
		/* value */ value,
		/* help  */ help,
		/* lazy  */ lazy,
		/*prepend*/ prepend
	);
	m->next = sc_dir (sc)->macros;
	sc_dir (sc)->macros = m;
}


#define IF_VAL 0x01
#define IF_HAS 0x02
#define MAX_IFSTACK 16

bool
walkifstack (s, n)
const char *s;
size_t n;
{
	size_t i;

	for (i = 0; i < n; ++i) {
		if (!(s[i] & IF_VAL))
			return false;
	}
	return true;
}

bool
is_directive (out, s, name)
char **out, *s;
const char *name;
{
	size_t len;

	if (*s != '.')
		return false;
	++s;

	skip_ws (&s);
	len = strlen (name);
	if (strncmp (s, name, len) != 0)
		return false;

	s += len;

	if (*s != '\0' && !isspace (*s))
		return false;

	if (out != NULL)
		*out = trim (s);
	return true;
}

bool
is_target (out, s, name)
char **out, *s;
const char *name;
{
	size_t len;

	len = strlen (name);
	if (strncmp (s, name, len) != 0)
		return false;

	s += len;
	skip_ws (&s);
	if (*s != ':')
		return false;
	++s;

	if (out != NULL)
		*out = trim (s);

	return true;
}

char *
parse_def (s, name, value)
char *s, **name, **value;
{
	char *i;

	for (i = s; *i != '\0'; ++i) {
		switch (*i) {
		case '$':
			switch (*++i) {
			case '\0':
				return NULL;
			case '{':
				for (++i; *i != '\0' && *i != '}'; ++i);
				if (*i != '}')
					return NULL;
				break;
			default:
				break;
			}
			break;
		case ':':
		case '=':
			*name = s;
			*value = i + 1;
			return i;
		}
	}
	return NULL;
}

void
do_parse (sc, dir, path, file)
struct scope *sc;
const struct path *dir;
const char *path;
FILE *file;
{
	extern void parse ();
	struct template *tm;
	struct rule *r = NULL;
	size_t len, cap, iflen = 0;
	char *s, *t, *u, *name, *value, *help = NULL;
	char ifstack[MAX_IFSTACK];
	int x, run, oldcline;
	FILE *tfile;
	str_t text;

	assert (sc->type == SC_DIR);
	cpath = path;

	if (verbose >= 3) {
		printf ("Parsing dir '%s' ...\n", path_to_str (dir));
	}

	cline = 1;

	for (; (s = readline (file, &cline)) != NULL; free (s)) {
		run = walkifstack (ifstack, iflen);
		if (s[0] == '#' && s[1] == '#') {
			help = expand (sc, dir, trim (s + 2), NULL);
			continue;
		} else if (s[0] == '#' || *trim (s) == '\0') {
			continue;
		} else if (starts_with (s, "include ")) {
			if (!run)
				goto cont;

			strip_comment (s);
			t = expand (sc, dir, trim (s + 8), NULL);
			if (*t == '/') {
				u = t;
			} else {
				u = strdup (path_cat_str (dir, t));
			}
			oldcline = cline;
			parse (sc, dir, u);
			sc_dir (sc)->done = 0;
			cline = oldcline;
			if (u != t)
				free (u);
			free (t);
		} else if (starts_with (s, "-include ") || starts_with (s, "sinclude ")) {
			if (!run)
				goto cont;

			strip_comment (s);
			t = expand (sc, dir, trim (s + 9), NULL);
			if (*t == '/') {
				u = t;
			} else {
				u = strdup (path_cat_str (dir, t));
			}

			if (access (t, R_OK) == 0)
				parse (sc, dir, u);

			sc_dir (sc)->done = 0;

			if (u != t)
				free (u);
			free (t);
		} else if (is_directive (&t, s, "include")) {
			if (!run)
				goto cont;

			errx (1, "%s:%d: please use .SUBDIRS: or .FOREIGN: now", path, cline);
		} else if (is_directive (&t, s, "if")) {
			if (iflen == MAX_IFSTACK)
				errx (1, "%s:%d: maximum .if depth of %d reached", path, cline, MAX_IFSTACK);
			x = parse_expr (sc, dir, t) & 0x01;
			ifstack[iflen++] = x * (IF_VAL | IF_HAS);
		} else if (is_directive (NULL, s, "else")) {
			if (iflen == 0)
				errx (1, "%s:%d: not in .if", path, cline);
			t = &ifstack[iflen - 1];
			*t = (!(*t & IF_HAS) * (IF_VAL | IF_HAS)) | (*t & IF_HAS);
		} else if (is_directive (&t, s, "elif")) {
			if (iflen == 0)
				errx (1, "%s:%d: not in .if", path, cline);
			x = parse_expr (sc, dir, t);
			t = &ifstack[iflen - 1];
			*t = ((!(*t & IF_HAS) && x) * (IF_VAL | IF_HAS)) | (*t & IF_HAS);
		} else if (is_directive (NULL, s, "endif")) {
			if (iflen == 0)
				errx (1, "%s:%d: not in .if", path, cline);
			--iflen;
		} else if (is_directive (&t, s, "template")) {
			str_new (&text);

			tm = new (struct template);
			tm->next = sc_dir (sc)->templates;
			tm->name = strdup (strip_comment (t));

			for (free (s); (s = readline (file, &cline)) != NULL; free (s)) {
				if (is_directive (NULL, s, "endt") || is_directive (NULL, s, "endtemplate"))
					break;

				str_puts (&text, s);
				str_putc (&text, '\n');
			}

			if (run) {
				tm->text = str_release (&text);
				sc_dir (sc)->templates = tm;
			} else {
				free (tm);
				str_free (&text);
			}
		} else if (is_directive (&t, s, "expand")) {
			if (!run)
				goto cont;
			tm = find_template (sc, strip_comment (t));
			if (tm == NULL)
				errx (1, "%s:%d: no such template: %s", cpath, cline, t);
			tfile = fmemopen (tm->text, strlen (tm->text), "r");
			do_parse (sc, dir, "template", tfile);
			fclose (tfile);
		} else if (is_target (&t, s, ".DEFAULT")) {
			if (run)
				sc_dir (sc)->default_file = strdup (strip_comment (t));
		} else if (is_target (NULL, s, ".POSIX")) {
			if (run)
				warnx ("%s:%d: this is not a POSIX-compatible make", path, cline);
		} else if (is_target (NULL, s, ".SUFFIXES")) {
			if (run)
				warnx ("%s:%d: this make doesn't require .SUFFIXES", path, cline);
		} else if (is_target (&t, s, ".SUBDIRS")) {
			if (run)
				parse_subdirs (sc, dir, t);
		} else if (is_target (&t, s, ".FOREIGN")) {
			if (run)
				parse_foreign (sc, t);
		} else if (is_target (&t, s, ".EXPORTS")) {
			if (run)
				parse_exports (sc, t);
		} else if (s[0] == '\t') {
			if (!run)
				goto cont;

			if (r == NULL)
				errx (1, "%s:%d: syntax error", path, cline);

			if (len == cap) {
				cap *= 2;
				r->code = renew (r->code, cap + 1, char *);
			}

			r->code[len++] = strdup (s + 1);
			r->code[len] = NULL;
		} else if ((t = parse_def (s, &name, &value)) != NULL) {
			if (!run)
				goto cont;

			if (trim (name) == t)
				goto invalid;

			switch (*t) {
			case ':':
				r = parse_rule (sc, dir, s, t, help);
				if (r == NULL)
					goto cont;

				len = 0;
				cap = 1;
				r->code = newa (cap + 1, char *);
				r->code[0] = NULL;
				break;
			case '=':
				/* TODO: write a proper function for checking macro names */
				if (*t == '\0')
					errx (1, "%s:%d: invalid macro name", path, cline);

				*t = '\0';

				parse_assign (sc, dir, s, t, help);
				break;
			default:
				abort ();
			}
		} else {
		invalid:
			warnx ("%s:%d: invalid line: %s", path, cline, s);
		}

	cont:
		help = NULL;
	}
}

void
parse (sc, dir, path) 
struct scope *sc;
const struct path *dir;
char *path;
{
	struct directory *dirx;
	FILE *file;

	file = fopen (path, "r");
	if (file == NULL) {
		/* err (1, "fopen(\"%s\")", path); */
		dirx = new (struct directory);
		dirx->subdirs = NULL;
		dirx->fhead = NULL;
		dirx->ftail = NULL;
		dirx->done = 0;
		sc->inner.dir = dirx;
		return;
	}

	if (sc->inner.dir == NULL) {
		dirx = new (struct directory);
		dirx->subdirs = NULL;
		dirx->fhead = NULL;
		dirx->ftail = NULL;
		dirx->done = 0;
		sc->inner.dir = dirx;
	} else if (sc_dir (sc)->done) {
		errx (1, "%s: parsing this file again?", path);
	}

	do_parse (sc, dir, path, file);
	sc_dir (sc)->done = 1;

	fclose (file);
}

void
parse_dir (sc, dir)
struct scope *sc;
const struct path *dir;
{
	char *path;

	path = strdup (path_cat_str (dir, sc->makefile));
	parse (sc, dir, path);
	free (path);
}

struct scope *
parse_recursive (dir, makefile)
const struct path *dir;
char *makefile;
{
	struct path *mfpath, *ppath;
	struct scope *sc, *parent;
	char *path, *name;

	tmppath.name = makefile;
	mfpath = path_cat (dir, &tmppath);
	if (access (path_to_str (mfpath), F_OK) != 0) {
		sc = NULL;
		goto ret;
	}

	ppath = path_cat (dir, &path_super);
	parent = parse_recursive (ppath, makefile);
	if (parent == NULL)
		parent = parse_recursive (ppath, MAKEFILE);
	free (ppath);

	name = path_basename (dir);

	if (parent != NULL) {
		if (parent->type != SC_DIR)
			errx (1, "%s: invalid parent type", path_to_str (mfpath));

		for (sc = sc_dir (parent)->subdirs; sc != NULL; sc = sc->next) {
			if (strcmp (sc->name, name) == 0) {
				if (sc->type != SC_DIR)
					errx (1, "%s: invalid type", path_to_str (mfpath));
				goto parselbl;
			}
		}

		goto create;
	} else {
	create:
		sc = new (struct scope);
		sc->type = SC_DIR;
		sc->name = name;
		sc->inner.dir = NULL;
	}

parselbl:
	sc->makefile = makefile;
	sc->parent = parent;
	path = strdup (path_to_str (mfpath));
	parse (sc, dir, path);
	free (path);

ret:
	free (mfpath);
	return sc;
}

struct path *	
parse_subdir (prefix, sub)
const struct path *prefix;
struct scope *sub;
{
	struct path *np;

	tmppath.name = sub->name;
	np = path_cat (prefix, &tmppath);
	parse_dir (sub, np);
	return np;
}

/* INFERENCE RULES */

char *
replace_suffix (name, sufx)
const char *name, *sufx;
{
	char *out, *ext;
	size_t len_name, len_sufx;

	ext = strrchr (name, '.');
	len_name = ext != NULL ? (size_t)(ext - name) : strlen (name);
	len_sufx = strlen (sufx);

	out = newa (len_name + len_sufx + 1, char);
	memcpy (out, name, len_name);
	memcpy (out + len_name, sufx, len_sufx);
	out[len_name + len_sufx] = '\0';

	return out;
}

/* instantiate a new file from an inference rule */
struct file *
inst_inf (sc, inf, name)
struct scope *sc;
struct inference *inf;
const char *name;
{
	struct file *f;
	struct dep *dep;

	dep = new_dep_name (replace_suffix (name, inf->from));

	f = new_file (
		/* name */ strdup (name),
		/* rule */ inf->rule,
		/* time */ time_zero,
		/* deps */ dep,
		/* dtail*/ dep,
		/* help */ NULL,
		/* inf  */ inf,
		/* obj  */ 0
	);
	dir_add_file (sc_dir (sc), f);

	return f;
}

/* instantiate an inference rule on an existing file */
void
inf_inst_file (f, inf)
struct file *f;
struct inference *inf;
{
	struct dep *dep;

	assert (f->inf == NULL);
	/* TODO: this is ugly */
	assert (f->rule == NULL || f->rule->code == NULL || *f->rule->code == NULL);

	dep = new_dep_name (replace_suffix (f->name, inf->from));

	/* prepend dependency to file */
	if (f->dhead != NULL) {
		dep->next = f->dhead;
		f->dhead->prev = dep;
	}
	f->dhead = dep;
	f->inf = inf;
	f->rule = inf->rule;
}

struct inference *
find_inf (sc, dir, name)
struct scope *sc;
const struct path *dir;
const char *name;
{
	extern struct file *try_find ();
	struct inference *inf;
	struct file *sf;
	char *sn, *base;
	const char *ext;

	ext = strrchr (name, '.');
	base = strdup (name);
	if (ext == NULL) {
		ext = "";
	} else {
		base[ext - name] = '\0';
	}

	for (inf = sc_dir (sc)->infs; inf != NULL; inf = inf->next) {
		if (strcmp (inf->to, ext) == 0) {
			sn = xstrcat (base, inf->from);
			sf = find_file (sc_dir (sc), sn);
			if (sf == NULL)
				sf = try_find (sc, dir, sn);
			free (sn);
			if (sf != NULL)
				break;
		}
	}

	free (base);
	return inf != NULL || sc->parent == NULL ? inf : find_inf (sc->parent, dir, name);
}

struct file *
try_find (sc, dir, name)
struct scope *sc;
const struct path *dir;
const char *name;
{
	struct inference *inf;
	struct filetime ft;
	struct file *f;

	get_mtime (&ft, sc, dir, name);
	if (tv_cmp (&ft.t, &time_zero) <= 0) {
		inf = find_inf (sc, dir, name);
		if (inf == NULL)
			return NULL;
		return inst_inf (sc, inf, name);
	}

	f = new_file (
		/* name */ strdup (name),
		/* rule */ NULL,
		/* time */ ft.t,
		/* deps */ NULL,
		/* dtail*/ NULL,
		/* help */ NULL,
		/* inf  */ NULL,
		/* obj  */ ft.obj
	);
	dir_add_file (sc_dir (sc), f);

	return f;
}

/* BUILDING */

struct build {
	struct timespec t;
	struct file *f;
	int obj;
};

void
build_init (out, t, f, obj)
struct build *out;
struct timespec t;
struct file *f;
{
	out->t = t;
	out->f = f;
	out->obj = obj;
}

int
build_deps (sc, dhead, prefix, mt, maxt, needs_update)
struct scope *sc;
struct dep *dhead;
const struct path *prefix;
struct timespec *mt, *maxt;
int *needs_update;
{
	extern int build_dir ();
	struct build b;
	struct dep *dep;
	int ec = 0;

	for (dep = dhead; dep != NULL; dep = dep->next) {
		if (build_dir (&b, sc, dep->path, prefix) == 0) {
			dep->obj = b.obj;

			if (tv_cmp (&b.t, mt) >= 0)
				*needs_update = 1;
			if (tv_cmp (&b.t, maxt) > 0)
				*maxt = b.t;
		} else {
			ec = 1;
			if (!conterr)
				break;
		}
	}

	return ec;
}

/* TODO: refactor this function, to be less complicated */
int
build_file (out, sc, name, prefix)
struct build *out;
struct scope *sc;
char *name;
const struct path *prefix;
{
	extern int build_dir ();
	int needs_update;
	struct scope *sub;
	struct path *new_prefix, xpath[2];
	struct file *f;
	struct timespec maxt;
	struct inference *inf;
	struct expand_ctx ctx;
	struct filetime ft;
	struct dep *dep, xdep;
	struct build b;
	char **s;
	int ec, rc;

	if (verbose >= 2) {
		printf ("dir %s", path_to_str (prefix));
		if (name)
			printf (" (%s)", name);
		printf (" ...\n");
	}

	if (!sc->created) {
		sc_mkdir_p (sc);
		sc->created = 1;
	}

	switch (sc->type) {
	case SC_DIR:
		/* lazily parse subdirectories */
		if (sc_dir (sc) == NULL)
			parse_dir (sc, prefix);

		/* if no rule specified to build, use the default rule */
		if (name == NULL && sc_dir (sc)->default_file != NULL)
			name = sc_dir (sc)->default_file;

		if (name != NULL) {
			/* try finding an explicitly defined file */
			f = find_file (sc_dir (sc), name);

			if (f == NULL) {
				/* try finding and building a subdirectory */
				sub = find_subdir (sc, name);
				if (sub != NULL) {
					tmppath.name = name;
					new_prefix = path_cat (prefix, &tmppath);
					ec = build_file (out, sub, NULL, new_prefix);
					free (new_prefix);
					return ec;
				}

				/* try finding an inference rule */
				f = try_find (sc, prefix, name);
				if (f == NULL)
					errx (1, "%s: no such file: %s", sc_path_str (sc), name);
			} else {
				get_mtime (&ft, sc, prefix, name);
				f->mtime = ft.t;
				f->obj = ft.obj;
			}
		} else {
			f = sc_dir (sc)->fhead;
			if (f == NULL)
				errx (1, "%s: nothing to build", sc_path_str (sc));
		}

		/* if this file has no rule, try to find an inference rule */
		if (f->rule == NULL || *f->rule->code == NULL) {
			/* try finding an inference rule */
			inf = name != NULL ? find_inf (sc, prefix, name) : NULL;

			if (inf != NULL) {
				/* instantiate inference rule */
				inf_inst_file (f, inf);
			} else if (f->rule == NULL) {
				if (tv_cmp (&f->mtime, &time_zero) > 0) {
					build_init (out, f->mtime, f, f->obj);
					return 0;
				} else {
					errx (1, "%s: no rule to build: %s", sc_path_str (sc), name);
				}
			}
		}

		needs_update = (tv_cmp (&f->mtime, &time_zero) <= 0);
		maxt = f->mtime;

		if (f->err)
			return 1;

		/* build dependencies and record timestamps */
		if (build_deps (sc, f->dhead, prefix, &f->mtime, &maxt, &needs_update) != 0) {
			f->err = 1;
			if (!conterr)
				return 1;
		}

		/* build dependencies from inference rule */
		if (f->inf != NULL) {
			if (build_deps (sc, f->inf->dhead, prefix, &f->mtime, &maxt, &needs_update) != 0) {
				f->err = 1;
				if (!conterr)
					return 1;
			}
		}

		if (!needs_update) {
			build_init (out, f->mtime, f, f->obj);
			return 0;
		}

		if (f->err)
			return 1;

		s = f->rule->code;

		/* rule is a "sum" rule, so doesn't need to be built */
		if (s == NULL || *s == NULL) {
			build_init (out, maxt, f, f->obj);
			return 0;
		}

		/* run commands */
		ectx_file (&ctx, sc, f);
		for (; *s != NULL; ++s) {
			if ((rc = runcom (sc, prefix, *s, &ctx, name)) != 0) {
				fprintf (stderr, "%s: command failed with %d: %s\n", sc_path_str (sc), rc, *s);
				f->err = 1;
				return 1;
			}
		}
		ectx_free (&ctx);

		/* update timestamp */
		get_mtime (&ft, sc, prefix, f->name);
		f->mtime = ft.t;
		f->obj = ft.obj;
		build_init (out, f->mtime, f, f->obj);
		return 0;
	case SC_CUSTOM:
		/* run the "subdir?" rule, to test if the target needs to be updated */
		new_prefix = path_cat (prefix, &path_super);
		if (name != NULL) {
			xpath[0].type = PATH_NAME;
			xpath[0].name = name;
			xpath[1].type = PATH_NULL;
		}

		xdep.next = xdep.prev = NULL;
		xdep.path = xpath;
		xdep.obj = 0;

		f = sc_custom (sc)->test;
		if (f != NULL) {
			assert (f->inf == NULL);

			ec = 0;
			for (dep = f->dhead; dep != NULL; dep = dep->next) {
				if (build_dir (&b, sc->parent, dep->path, new_prefix) != 0) {
					ec = 1;
					if (!conterr)
						return ec;
				}
			}

			if (ec != 0)
				return ec;

			ectx_init (
				/* ctx    */ &ctx, 
				/* target */ prefix[path_len (prefix) - 1].name,
				/* dep0   */ name != NULL ? &xdep : NULL,
				/* deps   */ f->dhead,
				/* infdeps*/ NULL
			);
			
			needs_update = 0;
			for (s = f->rule->code; *s != NULL; ++s) {
				if (runcom (sc->parent, new_prefix, *s, &ctx, name) != 0) {
					needs_update = 1;
					break;
				}
			}
		} else {
			needs_update = 1;
		}

		if (!needs_update) {
			if (name != NULL && get_mtime (&ft, sc, prefix, name) == 0) {
				build_init (out, ft.t, NULL, ft.obj);
			} else {
				build_init (out, time_zero, NULL, 0);
			}
			return 0;
		}

		/* run the "subdir!" rule */
		f = sc_custom (sc)->exec;
		if (f == NULL)
			errx (1, "%s: missing '%s!' rule", sc_path_str (sc->parent), sc->name);
		assert (f->inf == NULL);

		ectx_init (
			/* ctx    */ &ctx, 
			/* target */ prefix[path_len (prefix) - 1].name,
			/* dep0   */ name != NULL ? &xdep : NULL,
			/* deps   */ f->dhead,
			/* infdeps*/ NULL
		);

		ec = 0;
		for (dep = f->dhead; dep != NULL; dep = dep->next) {
			if (build_dir (&b, sc->parent, dep->path, new_prefix) != 0) {
				ec = 1;
				if (!conterr)
					return ec;
			}
		}
		if (ec != 0)
			return ec;

		for (s = f->rule->code; *s != NULL; ++s) {
			if ((rc = runcom (sc->parent, new_prefix, *s, &ctx, name)) != 0) {
				fprintf (stderr, "%s: command failed with %d: %s\n", sc_path_str (sc->parent), rc, *s);
				return 1;
			}
		}

		free (new_prefix);
		if (name != NULL && get_mtime (&ft, sc, prefix, name) == 0) {
			build_init (out, ft.t, NULL, ft.obj);
		} else {
			build_init (out, now (), NULL, 0);
		}
		return 0;
	}
	
	abort ();
}

int
build_dir (out, sc, path, prefix)
struct build *out;
struct scope *sc;
const struct path *path, *prefix;
{
	struct path *new_prefix;
	struct scope *sub;
	int ec;

	switch (path[0].type) {
	case PATH_SUPER:
		new_prefix = path_cat (prefix, &path[0]);
		ec = build_dir (out, sc->parent, path + 1, new_prefix);
		free (new_prefix);
		return ec;
	case PATH_NULL:
		return build_file (out, sc, NULL, prefix);
	case PATH_NAME:
		if (path[1].type == PATH_NULL)
			return build_file (out, sc, path[0].name, prefix);

		if (sc->type != SC_DIR)
			errx (1, "%s: invalid path", sc_path_str (sc));

		if (sc_dir (sc) == NULL)
			parse_dir (sc, prefix);

		new_prefix = path_cat (prefix, &path[0]);

		sub = find_subdir (sc, path[0].name);
		if (sub == NULL)
			errx (1, "%s: invalid subdir: %s", sc_path_str (sc), path[0].name);

		ec = build_dir (out, sub, path + 1, new_prefix);
		free (new_prefix);
		return ec;
	}

	abort ();
}

int
build (out, sc, path)
struct build *out;
struct scope *sc;
const struct path *path;
{
	return build_dir (out, sc, path, &path_null);
}

/* HELP */

void
help_macros (sc)
struct scope *sc;
{
	struct macro *m;

	assert (sc_dir (sc) != NULL);

	for (m = sc_dir (sc)->macros; m != NULL; m = m->next) {
		if (m->help == NULL)
			continue;

		printf ("  %-28s- %s\n", m->name, m->help);
	}
}

void
help_files (prefix, sc)
const struct path *prefix;
struct scope *sc;
{
	struct path *new_prefix;
	struct scope *sub;
	struct file *f;
	const char *p;
	int n;

	p = path_to_str (prefix);
	if (strcmp (p, ".") == 0)
		p = NULL;

	for (f = sc_dir (sc)->fhead; f != NULL; f = f->next) {
		if (f->help == NULL)
			continue;

		n = 0;
		if (p != NULL) {
			printf ("%s/", p);
			n += strlen (p) + 1;
		}
		printf ("  %s", f->name);
		n += strlen (f->name);
		printf ("%-*s- %s\n", n < 28 ? 28 - n : 0, "", f->help);
	}

	if (!verbose)
		return;

	for (sub = sc_dir (sc)->subdirs; sub != NULL; sub = sub->next) {
		if (sub->type != SC_DIR)
			continue;

		new_prefix = parse_subdir (prefix, sub);
		help_files (new_prefix, sub);
		free (new_prefix);
	}
}

int
help (prefix, sc)
const struct path *prefix;
struct scope *sc;
{
	extern int usage ();
	usage (1);

	fputs ("\nOptions:\n", stderr);
	fputs ("  -C dir                      - chdir(dir)\n", stderr);
	fprintf (stderr, "  -f file                     - read `file` instead of \"%s\"\n", MAKEFILE);
	fputs ("  -o objdir                   - put build artifacts into objdir\n", stderr);
	fputs ("  -V var                      - print expanded version of var\n", stderr);
	fputs ("  -h                          - print help page\n", stderr);
	fputs ("  -hv                         - print help page, recursively\n", stderr);
	fputs ("  -p                          - dump tree\n", stderr);
	fputs ("  -pv                         - dump tree, recursively\n", stderr);
	fputs ("  -s                          - do not echo commands\n", stderr);
	fputs ("  -k                          - continue processing after errors are encountered\n", stderr);
	fputs ("  -S                          - stop processing when errors are encountered (default)\n", stderr);
	fputs ("  -v                          - verbose output\n", stderr);

	if (sc != NULL) {
		fputs ("\nMacros:\n", stderr);
		help_macros (sc);

		fputs ("\nTargets:\n", stderr);
		help_files (prefix, sc);
	}

	fprintf (stderr, "\nbmk v%s, send bug reports to <%s>.\n", BMK_VERSION, BMK_EMAIL);
	return 1;
}

/* DUMP */

void
print_sc (prefix, sc)
const struct path *prefix;
struct scope *sc;
{
	struct path *new_prefix;
	struct inference *inf;
	struct scope *sub;
	struct macro *m;
	struct dep *dep;
	struct file *f;
	struct rule *r;
	char **s;

	if (verbose)
		printf ("=== %s\n", sc_path_str (sc));

	if (sc->type != SC_DIR || sc_dir (sc) == NULL)
		errx (1, "%s: print_sc(): must be of type SC_DIR", sc_path_str (sc));

	if (sc_dir (sc)->default_file != NULL)
		printf (".DEFAULT: %s\n", sc_dir (sc)->default_file);

	for (m = sc_dir (sc)->macros; m != NULL; m = m->next) {
		if (m->help != NULL)
			printf ("\n## %s\n", m->help);
		printf ("%s %s= %s\n", m->name, m->prepend != NULL ? "+" : "", m->value);
	}

	printf ("\n");

	for (f = sc_dir (sc)->fhead; f != NULL; f = f->next) {
		if (f->help != NULL)
			printf ("## %s\n", f->help);
		printf ("%s:", f->name);
	
		for (dep = f->dhead; dep != NULL; dep = dep->next)
			printf (" %s", path_to_str (dep->path));
		if (f->inf != NULL) {
			for (dep = f->inf->dhead; dep != NULL; dep = dep->next)
				printf (" %s", path_to_str (dep->path));
		}
		printf ("\n");

		r = f->rule;
		if (r != NULL && r->code != NULL) {
			for (s = r->code; *s != NULL; ++s)
				printf ("\t%s\n", *s);
		}
		printf ("\n");
	}

	for (inf = sc_dir (sc)->infs; inf != NULL; inf = inf->next) {
		printf ("%s%s:", inf->from, inf->to);
		for (dep = inf->dhead; dep != NULL; dep = dep->next)
			printf (" %s", path_to_str (dep->path));
		printf ("\n");
		if (inf->rule->code != NULL) {
			for (s = inf->rule->code; *s != NULL; ++s)
				printf ("\t%s\n", *s);
			printf ("\n");
		}
	}
	
	for (sub = sc_dir (sc)->subdirs; sub != NULL; sub = sub->next) {
		switch (sub->type) {
		case SC_DIR:
			printf (".include %s, DIR", sub->name);
			if (sc->makefile != NULL)
				printf (", %s", sc->makefile);
			printf ("\n");
			break;
		case SC_CUSTOM:
			printf (".include %s, CUSTOM\n", sub->name);
			break;
		}
	}

	if (verbose) {
		for (sub = sc_dir (sc)->subdirs; sub != NULL; sub = sub->next) {
			if (sub->type != SC_DIR)
				continue;

			printf ("\n");

			new_prefix = parse_subdir (prefix, sub);
			print_sc (new_prefix, sub);
			free (new_prefix);
		}
	}
}

/* MAIN */

int
usage (uc)
{
	fprintf (stderr, "%s: %s [-hkpsSv] [-C dir] [-f makefile] [-o objdir] [-V var] [target...]\n", uc ? "USAGE" : "usage", m_make.value);
	return 1;
}

void
do_V (sc, V)
struct scope *sc;
const char *V;
{
	size_t len;
	char *s;

	if (strchr (V, '$') != 0) {
		s = strdup (V);
	} else {
		len = strlen (V);
		s = newa (len + 4, char);
		s[0] = '$';
		s[1] = '{';
		memcpy (s + 2, V, len);
		s[len + 2] = '}';
		s[len + 3] = '\0';
	}

	puts (expand (sc, &path_null, s, NULL));
	free (s);
}

int
main (argc, argv)
char **argv;
{
	extern char *optarg;
	extern int optind;
	str_t cmdline;
	struct scope *sc;
	struct path *path;
	struct macro *m;
	struct build b;
	char *s, *cd = NULL, *makefile = MAKEFILE, *V = NULL, *odir = NULL;
	int i, option, pr = 0, n = 0, dohelp = 0;

	m_dmake.value = m_make.value = argv[0];

	str_new (&cmdline);
	while ((option = getopt (argc, argv, "hpsvkSC:f:V:o:")) != -1) {
		switch (option) {
		case 'h':
			dohelp = 1;
			break;
		case 'p':
			str_puts (&cmdline, " -p");
			pr = 1;
			break;
		case 's':
			str_puts (&cmdline, " -s");
			verbose = -1;
			break;
		case 'v':
			str_puts (&cmdline, " -v");
			++verbose;
			break;
		case 'C':
			cd = optarg;
			break;
		case 'f':
			makefile = optarg;
			break;
		case 'V':
			V = optarg;
			break;
		case 'o':
			odir = optarg;
			break;
		case 'k':
			conterr = 1;
			break;
		case 'S':
			conterr = 0;
			break;
		case '?':
			return usage (0);
		default:
			errx (1, "unexpected option: -%c", option);
		}
	}

	if (odir != NULL) {
		mkdir_p (odir);
		objdir = realpath (odir, newa (PATH_MAX, char));
		if (objdir == NULL)
			err (1, "realpath()");

		if (verbose >= 3)
			printf ("objdir = '%s'\n", objdir);
	}

	if (cd != NULL && chdir (cd) != 0)
		err (1, "chdir()");

	argv += optind;
	argc -= optind;

	for (i = 0; i < argc; ++i) {
		s = strchr (argv[i], '=');
		if (s == NULL)
			continue;

		str_putc (&cmdline, ' ');
		str_puts (&cmdline, argv[i]);

		*s = '\0';

		m = new_macro (
			/* name  */ trim (argv[i]),
			/* value */ trim (s + 1),
			/* help  */ NULL,
			/* lazy  */ 0,
			/*prepend*/ NULL
		);
		m->next = globals;
		globals = m;

		argv[i] = NULL;
	}

	str_trim (&cmdline);
	m_dmakeflags.value = m_makeflags.value = str_release (&cmdline);

	path = parse_path (".");
	sc = parse_recursive (path, makefile);

	if (dohelp)
		return help (path, sc);

	if (sc == NULL)
		errx (1, "failed to find or parse makefile");


	if (pr) {
		print_sc (path, sc);
		return 0;
	}

	if (V != NULL) {
		do_V (sc, V);
		return 0;
	}

	free (path);

	for (i = 0; i < argc; ++i) {
		if (argv[i] == NULL)
			continue;

		path = parse_path (argv[i]);
		if (build (&b, sc, path) != 0)
			return 1;
		free (path);
		++n;
	}

	return n == 0 ? build (&b, sc, &path_null) : 0;
}

