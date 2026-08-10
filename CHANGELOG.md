# Changelog

All notable changes to `bmk` (the `mk` binary) are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- New `mk-timings-summary` script that reads a timings file produced by
  `-t` and prints a table of total wall-clock time and command count per
  rule, sorted slowest-first, with a grand-total row. Accepts a filename
  argument or reads from standard input. Installed alongside `mk`.
- New `-t file` option to record build timings. After each shell command
  completes, a line of the form `RULE,COMMAND,TIME_S,TIME_NS` is appended
  to `file`. Using `-t -` writes to standard output. When `file` is a
  regular file (not `-`), a CSV header line is written first. `COMMAND`
  is the last field so that embedded commas do not break parsing.

### Fixed
- `$@` is now set to an empty string (instead of being left as a null
  pointer) when building the default rule of a `.FOREIGN:` subdirectory,
  preventing a potential null-dereference.
- The `<` and `>` comparison operators in `.if` conditions no longer
  advance the parse pointer an extra time when the next character is not
  `=`, fixing incorrect evaluation of bare `<` / `>` comparisons.
- `now()` no longer copies `tv_sec` into `tv_nsec` when falling back to
  `gettimeofday(2)`; nanoseconds are now correctly derived from
  `tv_usec * 1000`.

### Portability
- `toupper()`/`tolower()` are now wrapped in `mk_toupper()`/`mk_tolower()`
  to avoid the unconditional macro definitions on 2.11BSD and 4.3BSD,
  which mangled characters outside `a-z`/`A-Z` and broke the `:U` and `:L`
  macro modifiers on those systems.
- `F_OK`, `X_OK`, `W_OK`, and `R_OK` are now defined directly in
  `compats.h` instead of conditionally including `<sys/file.h>`, which
  declares its own `struct file` and collided with bmk's internal type.
- Replaced all `#elif` directives with stacked `#if`/`#endif` blocks for
  compatibility with primitive preprocessors (2.11BSD, XENIX).

## [0.3] - 2026-08-06

### Added
- `${.SCOPE}` dynamic variable, which expands to the path of the current
  scope relative to the top-level directory, plus the `$&` shortcut for
  `${.SCOPE:T}` (its basename).
- Parenthesised sub-expressions in `.if` conditions, e.g.
  `.if (a == b || c == d) && defined(X)`.
- Backtick command substitution in `.if` conditions:
  `` .if `uname -s` == "OpenBSD" ``.
- Macro references are now expanded in the directory lists of
  `.SUBDIRS:` and `.FOREIGN:`, so `.SUBDIRS: ${SUBDIRS}` is accepted.
- Subdirectories referenced as prerequisites are now resolved
  automatically: directories declared via `.SUBDIRS:` are given a lazy
  scope and their targets may be built, while undeclared directories that
  exist on the filesystem are stat'd directly (their modification time is
  used, but their makefile is not parsed and no build rules are applied).
- Bare `.FOREIGN:` subdirectory names may be used as targets to declare
  build ordering between foreign subdirectories without supplying a recipe.

### Changed
- Shell command substitution (the `!=` assignment operator and `.if`
  backticks) now aborts the build on non-zero exit status instead of
  silently continuing.
- for `.FOREIGN:` subdirectories: `$<` is now `$@`, and `$@` is not `$&`

## [0.2] - 2026-07-21

### Added
- Syntax highlighting file for Vim (`bmk.vim`).
- User-defined `${SHELL}` macro to select the recipe shell.

### Changed
- The command echo no longer prints a stray path prefix when running at the
  top-level scope.

### Fixed
- Fixed a crash when a single dependency line listed the same target more
  than once (e.g. `a a a: c`); each target now gets its own copy of the
  dependency list.
- Duplicate pre-existing targets on a rule line no longer rebuild shared
  prerequisites multiple times.

## [0.1] - 2025-05-16

Initial release.

### Added
- Scope-based build model: every directory is a scope arranged in a tree,
  eliminating recursive `make` invocations while keeping per-directory
  state. Targets and prerequisites are treated as paths and parsed lazily
  per subtree.
- Inference rules (`.from.to`) with arbitrarily deep chains and no
  `.SUFFIXES:` declaration required.
- Macros with the operators `=`, `:=`, `::=`, `+=`, `?=`, `??=`, and `!=`.
- Macro modifiers: `:U`, `:L`, `:F`, `:E`, `:R`, `:H`, `:T`, `:Mpattern`,
  `:Npattern`, `:Jseparator`, and `:old=new`.
- Special variables `.TARGET` (`$@`), `.IMPSRC` (`$<`), `.ALLSRC` (`$^`),
  `.IMPSRC:T` (`$*`), `.TOPDIR` (`$.`), `.OBJDIR`, `.SUBDIRS`,
  `.EXPORTS`, `.MAKEFILES`, `SHELL`, and `MAKE`, plus `$N` positional
  variables.
- Special targets `.DEFAULT`, `.SUBDIRS`, `.FOREIGN`, `.EXPORTS`,
  `.POSIX`, and `.SUFFIXES`.
- `.if` / `.elif` / `.else` / `.endif` conditionals with `defined()`
  and `target()` tests.
- `include`, `-include`, and `sinclude` statements.
- `.template` / `.endt` / `.expand` text templates.
- Documentation comments (`##`) surfaced by `mk -h`.
- Out-of-tree builds via `-o` / `${.OBJDIR}`.
- Foreign subdirectory integration driven by external build systems
  (GNU make, Autotools, BSD make) via `.FOREIGN:` and `.EXPORTS:`.
- Hand-written, portable `configure` script.
- Extreme portability: builds from modern OpenBSD/macOS down to 2.11BSD,
  4.3BSD, XENIX, and Minix.

[Unreleased]: https://github.com/realchonk/bmk/compare/0.3...HEAD
[0.3]: https://github.com/realchonk/bmk/compare/0.2...0.3
[0.2]: https://github.com/realchonk/bmk/compare/0.1...0.2
[0.1]: https://github.com/realchonk/bmk/releases/tag/0.1

