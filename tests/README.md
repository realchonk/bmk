# bmk test suite

A portable POSIX-`sh` harness for `mk(1)`. There is one `test_*.sh` file per
feature area; each file sources `common.sh`, runs a number of assertions, and
exits non-zero on the first file that has any failing assertion.

## Running

From the repository root, after building `mk`:

```sh
./configure && make        # build ./mk
make check                 # run the whole suite (alias: make test)
```

or run the harness directly:

```sh
sh tests/run.sh                       # uses ./mk
MK=/usr/local/bin/mk sh tests/run.sh  # test an installed binary
```

`run.sh` discovers every `tests/test_*.sh`, runs it in its own shell, and
prints `FILE <name> PASS|FAIL` plus a final summary. The overall exit status is
non-zero if any assertion failed.

## Layout

| file                    | covers                                             |
|-------------------------|----------------------------------------------------|
| `common.sh`             | assertion helpers, temp-dir + `mk` wrapper         |
| `run.sh`                | discovers and runs the `test_*.sh` files           |
| `test_cli.sh`           | `-h -V -f -C -o -s -k -S -p`, default goal, etc.   |
| `test_assignments.sh`   | `= := ::= += ?= ??= !=`, lazy vs immediate         |
| `test_modifiers.sh`     | `:U :L :F :E :R :H :T :M :N :J :old=new`, chaining |
| `test_special_vars.sh`  | `$@ $< $^ $& $. .SCOPE .OBJDIR .EXPORTS ...`      |
| `test_expansion.sh`     | `${X}`, `$X`, `$$`, undefined macros, continuations|
| `test_conditionals.sh`  | `.if/.elif/.else/.endif`, `defined`, `target`, `` `cmd` ``, `() && || !` |
| `test_inference.sh`     | `.from.to:` rules, `$<`/`$@`, deep chains          |
| `test_includes.sh`      | `include`, `-include`/`sinclude`, expansion        |
| `test_subdirs.sh`       | `.SUBDIRS`, lazy parsing, `${.SUBDIRS:=/goal}`     |
| `test_templates.sh`     | `.template/.endt`, `.expand`, parameterisation     |
| `test_recipes.sh`       | `@`/`-` prefixes, echo format, `-v`, fresh shells  |
| `test_build.sh`         | incremental rebuilds, up-to-date detection         |
| `test_foreign.sh`       | `.FOREIGN`, `.EXPORTS`, `?`/`!` hooks, ordering    |
| `test_comments.sh`      | `#` vs `##`, doc-comment expansion                 |
| `test_errors.sh`        | missing makefile/prereq, invalid lines, warnings   |

## Writing a new test

```sh
#!/bin/sh
. "$TESTDIR/common.sh"

begin "my feature"
setup                       # fresh temp dir, cwd into it
cat > Mkfile <<'EOF'
all:
	@echo hi
EOF
mkrun                       # sets $OUT, $ERR, $RC
eq "$OUT" "hi" "the recipe ran"
contains "$ERR" "something" "stderr carried the expected message"
finish
```

Available assertions: `eq`, `neq`, `rc_ok`, `rc_fail`, `rc_is`, `contains`,
`absent`, `matches`, `file_exists`. See `common.sh` for the full list.
