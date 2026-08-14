#ifndef FILE_MAKE_H
#define FILE_MAKE_H

enum path_type {
	PATH_NULL,
	PATH_SUPER,
	PATH_NAME,
};
struct path {
	enum path_type		 type;
	char			*name;
};

struct template {
	SLIST_ENTRY(template)	 next;
	char			*name;
	char			*text;
};
SLIST_HEAD(template_list, template);

enum scope_type {
	SC_DIR,
	SC_FOREIGN,
};

struct scope {
	SLIST_ENTRY(scope)	 next;
	enum scope_type		 type;
	char			*name;		/* optional */
	struct scope		*parent;	/* optional */
	char			*makefile;	/* required */
	bool			 created;	/* mkdir() */
	union {
		struct directory	*dir;	/* optional */
		struct foreign		*foreign; /* required */
	} inner;
};
SLIST_HEAD(scope_list, scope);

/*
 * struct dep: doubly-linked, always owned by exactly one struct file or
 * struct foreign (via dhead/dtail).  Ownership is expressed as a TAILQ.
 */
struct dep {
	TAILQ_ENTRY(dep)	 link;
	struct path		*path;
	bool			 obj;
};
TAILQ_HEAD(dep_list, dep);

enum file_state {
	FILE_PENDING,	/* not yet started */
	FILE_BUSY,	/* currently building */
	FILE_DONE,	/* built */
};

/*
 * struct file: doubly-linked, owned by struct directory (fhead/ftail).
 */
struct file {
	TAILQ_ENTRY(file)	 link;
	char			*name;
	struct rule		*rule;	/* optional */
	struct dep_list		 deps;
	struct inference	*inf;	/* optional */
	struct timespec		 mtime;
	char			*help;	/* optional */
	bool			 obj;
	bool			 err;
	enum file_state		 state;
};
TAILQ_HEAD(file_list, file);

struct inference {
	SLIST_ENTRY(inference)	 next;
	char			*from;
	char			*to;
	struct rule		*rule;
	struct dep_list		 deps;
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
	SLIST_ENTRY(macro)	 next;
	SLIST_ENTRY(macro)	 enext;
	struct macro		*prepend;	/* prepend the value of this macro (+=) (optional) */
	char			*name;		/* required */
	char			*value;		/* required */
	char			*help;		/* optional */
	bool			 lazy;		/* the value is not yet expanded */
};
SLIST_HEAD(macro_list, macro);

struct directory {
	struct scope_list	 subdirs;	/* sub directories list */
	struct file_list	 files;		/* file list */
	struct macro_list	 macros;	/* macro list */
	struct macro_list	 emacros;	/* exported macros list */
	struct inference_list	 infs;		/* inference rules */
	struct template_list	 templates;	/* list of templates */
	char			*default_file;	/* default makefile name */
	bool			 done;		/* directory makefile is parsed */
};

struct cbuilt {
	SLIST_ENTRY(cbuilt)	 next;
	char			*name;
	struct timespec		 t;
	bool			 obj;
};
SLIST_HEAD(cbuilt_list, cbuilt);

struct foreign {
	struct file		*test;	/* rule to test if a target is up-to-date (optional) */
	struct file		*exec;	/* rule to build a target */
	struct dep_list		 deps;	/* ordering deps (bare name: target) */
	struct cbuilt_list	 built;	/* targets already built this run */
};

struct rule {
	char **code; /* optional */
};

#endif /* FILE_MAKE_H */
