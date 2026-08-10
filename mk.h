#ifndef FILE_MAKE_H
#define FILE_MAKE_H

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
	SLIST_ENTRY(template) next;
	char *name;
	char *text;
};
SLIST_HEAD(template_list, template);

enum scope_type {
	SC_DIR,
	SC_CUSTOM,
};

struct scope {
	SLIST_ENTRY(scope) next;
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
SLIST_HEAD(scope_list, scope);

/*
 * struct dep: doubly-linked, always owned by exactly one struct file or
 * struct custom (via dhead/dtail).  Ownership is expressed as a TAILQ.
 */
struct dep {
	TAILQ_ENTRY(dep) link;
	struct path *path;
	int obj;
};
TAILQ_HEAD(dep_list, dep);

/*
 * struct file: doubly-linked, owned by struct directory (fhead/ftail).
 */
struct file {
	TAILQ_ENTRY(file) link;
	char *name;
	struct rule *rule; /* optional */
	struct dep_list deps;
	struct inference *inf; /* optional */
	struct timespec mtime;
	char *help; /* optional */
	int obj, err;
	int built;
};
TAILQ_HEAD(file_list, file);

struct inference {
	SLIST_ENTRY(inference) next;
	char *from, *to;
	struct rule *rule;
	struct dep_list deps;
};
SLIST_HEAD(inference_list, inference);

/*
 * struct macro: participates in two independent singly-linked lists:
 *   - `next`  : the per-scope macro list (struct directory.macros)
 *   - `enext` : the exported-macro list  (struct directory.emacros)
 * `prepend` is a plain pointer (not a list head) to a macro whose value
 * is prepended (+=) to this one's value at expansion time.
 */
struct macro {
	SLIST_ENTRY(macro) next;
	SLIST_ENTRY(macro) enext;
	struct macro *prepend;
	char *name; /* required */
	char *value; /* required */
	char *help; /* optional */
	int lazy;
};
SLIST_HEAD(macro_list, macro);

struct directory {
	struct scope_list subdirs;
	struct file_list files;
	struct macro_list macros;
	struct macro_list emacros;	/* exported macros */
	struct inference_list infs;
	struct template_list templates;
	char *default_file;
	int done;
};

struct cbuilt {
	SLIST_ENTRY(cbuilt) next;
	char *name;
	struct timespec t;
	int obj;
};
SLIST_HEAD(cbuilt_list, cbuilt);

struct custom {
	struct file *test, *exec;
	struct dep_list deps; /* ordering deps (bare name: target) */
	struct cbuilt_list built; /* targets already built this run */
};

struct rule {
	char **code; /* optional */
};

#endif /* FILE_MAKE_H */
