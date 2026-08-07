# AGENTS.md

`bmk` (binary name `mk`) is a portable, scope-based reimplementation of
`make(1)` in C. The whole tool is essentially `mk.c` (~3700 lines) plus a
portability shim (`compats.c` / `compats.h`). Its defining constraint is
**extreme portability**: it must compile from modern OpenBSD/macOS down to
2.11BSD and XENIX, including pre-ANSI K&R compilers. Most non-obvious rules
below exist to satisfy that constraint.

## Version control: `got`, not git

This repo uses **Game of Trees (`got`)** — the `.got/` metadata is present and
`.gitignore` exists only for the GitHub mirror. Use `got status`, `got diff`,
`got log`, `got commit -m "..."`. Do not run git commands or add git plumbing.

## Build and "test"

There is **no test harness**. Correctness is verified by bootstrapping and
self-hosting across platforms. CI does this on Alpine/FreeBSD/NetBSD/OpenBSD
(`.builds/*.yml`, sourcehut) and macOS (`.cirrus.yml`); `retroci` ships a
tarball to real 2.11BSD/4.3BSD machines over telnet.

```sh
./configure [--prefix=...]   # hand-written (no autoconf); writes config.h, config.mk, Makefile, compile_flags.txt
make                         # bootstrap build with the system make -> ./mk
make install
make distclean               # remove all generated files above
```

Self-test the freshly built tool (this is the CI pattern — note that `clean`
deletes `mk`, so run it from a copy):

```sh
cp mk xmk && ./xmk clean all
./configure && mk && mk install    # full self-hosting loop
CC=tcc ./configure                 # exercises the tcc path (see .builds/openbsd.yml)
```

For ad-hoc behavior checks, write a small makefile and run `./mk -f <file>`.
`-p` / `-pv` dump the parsed scope/file tree (fastest way to see how a
makefile was interpreted). `mk` echoes each command as `[scope] $ ...`, where
an empty `[]` means the top-level scope.

## Portability rules (most code here exists for these)

- **K&R-style function definitions**, not ANSI prototypes — keep this for any
  function that must build on ancient compilers:
  ```c
  char *
  rtrim (s)
  	char *s;
  {
  	 ...
  }
  ```
- **Build flags intentionally tolerate old dialects** (`-ansi
  -Wno-deprecated-non-prototype -Wno-implicit-int -Wno-return-type`). Do not
  "modernize" them.
- **Feature-gated includes.** Never include a non-standard header
  unconditionally; wrap it in `#if HAVE_*` driven by `config.h`.
- **Probe in `./configure`, fall back in `compats.{c,h}`.** When you call a
  function that may be missing on old systems, add a feature probe in
  `./configure` (which emits the matching `HAVE_*` macro) and a guarded
  fallback here. Existing fallbacks include `reallocarray`, `strsep`,
  `strdup`, `err`/`warn`, `fnmatch`, `basename`/`dirname`, `struct timespec`,
  `realpath`, `WIFEXITED`/`WEXITSTATUS`, `PATH_MAX`, `lstat`.
- **`FIELD(name, value)` macro** (`mk.c`) abstracts C99 designated
  initializers for old compilers — use it for any static struct initializer.
- **`void_t`** (from `compats.h`) is `void` on conforming compilers and
  `char` otherwise; `void` itself may be `#define`d away on pre-ANSI
  compilers.
- **Declarations at the start of a block/function** (C89), never mid-block —
  a stated portability goal.
- **No `#elif`.** The target preprocessors are too primitive to support it.
  Use stacked `#if`/`#endif` instead (with `&&` / `!defined(...)` guards to
  make the branches mutually exclusive — see `now()` in `mk.c`).
- **`compats.c` deliberately does not include `compats.h`.** The shared
  macros (`void_t`, the `void`-redefinition, `__dead`, `PATH_MAX`) are
  duplicated across the two files on purpose; don't "deduplicate" by adding
  the include.

## Architecture: scopes, not strings

bmk's core difference from make is that **targets/dependencies are paths
(not strings) and every directory is a scope** arranged in a tree — this
avoids both recursive make and one giant Makefile. All core data structures
live in `mk.h` (`scope`, `directory`, `file`, `dep`, `inference`, `macro`,
`path`). A scope is either `SC_DIR` (holding a `struct directory`) or
`SC_CUSTOM` (wrapping a foreign GNU/BSD build system via a `test`/`exec`
file pair). Evaluation flow (also mapped in `TODO.md`): bottom-up
`parse_recursive()`, top-down `parse()`, lazy includes, `expand()` for macro
expansion, then `build()` / `build_file()`. Inference rules can be
arbitrously deep (no `.SUFFIXES:` needed).

## Editing conventions

- Most functions take an explicit `struct scope *sc` and/or
  `struct expand_ctx *ctx` — thread these through; do not add new globals.
  The few genuine globals (`cpath`, `objdir`, `verbose`, the `globals` macro
  list) sit near the top of `mk.c`.
- **`struct dep` nodes are intrusively linked and must be owned by exactly
  one `struct file`.** `file_add_deps()` asserts `dhead->prev == NULL` /
  `dtail->next == NULL`, so when one rule line lists several targets,
  deep-copy the dep list per target via `dup_deps()`. The `struct path`
  payloads, by contrast, may be shared — paths are never freed.
- `parse_rule()` returns the `struct rule *` that subsequent tab-indented
  command lines attach to (or `NULL` if the line introduced no rule body).
- **Update `mk.1`** (mandoc) whenever you change user-visible behavior,
  directives, or CLI options. The implemented special targets are
  `.DEFAULT`, `.SUBDIRS`, `.FOREIGN`, `.EXPORTS`, `.POSIX`, `.SUFFIXES`
  (the last two only emit warnings); `.TOP:` / `.PATH:` in `TODO.md` are
  future ideas, not implemented.
- **Read `TODO.md` before large changes** — it holds the roadmap, intended
  direction, known bugs, and reproducers.
- `bmk.typ` (Typst slides) and `bmk.vim` (WIP syntax highlighting) are
  auxiliary and not part of the build.
