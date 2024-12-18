#ifndef FILE_MAKE_H
#define FILE_MAKE_H
#include <sys/time.h>

enum path_type {
	PATH_NULL,
	PATH_SUPER,
	PATH_NAME,
};
struct path {
	enum path_type type;
	char *name;
};

struct template {
	struct template *next;
	char *name;
	char *text;
};

enum scope_type {
	SC_DIR,
	SC_CUSTOM,
};
struct scope {
	struct scope *next;
	enum scope_type type;
	char *name; /* optional */
	struct scope *parent; /* optional */
	char *makefile; /* required */
	int created;
	union {
		struct directory *dir; /* optional */
		struct custom *custom; /* required */
	} inner;
};

struct directory {
	struct scope *subdirs;
	struct file *fhead, *ftail;
	struct macro *macros;
	struct macro *emacros;	/* exported macros */
	struct inference *infs;
	struct template *templates;
	char *default_file;
	int done;
};

struct custom {
	struct file *test, *exec;
};

struct dep {
	struct dep *next, *prev;
	struct path *path;
	int obj;
};

struct file {
	struct file *next, *prev;
	char *name;
	struct rule *rule; /* optional */
	struct dep *dhead, *dtail;
	struct inference *inf; /* optional */
	struct timespec mtime;
	char *help; /* optional */
	int obj, err;
};

struct inference {
	struct inference *next;
	char *from, *to;
	struct rule *rule;
	struct dep *dhead, *dtail;
};

struct rule {
	char **code; /* optional */
};

struct macro {
	struct macro *next, *enext, *prepend;
	char *name; /* required */
	char *value; /* required */
	char *help; /* optional */
	int lazy;
};

#endif /* FILE_MAKE_H */
