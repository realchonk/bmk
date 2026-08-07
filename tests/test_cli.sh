#!/bin/sh
. "$TESTDIR/common.sh"

begin "CLI: -V prints a macro value"
setup
cat > Mkfile <<'EOF'
GREETING = hello-world
EOF
mkV GREETING
eq "$OUT" "hello-world" "-V NAME"
rc_ok "-V exits 0"

begin "CLI: -V with a \$ expands verbatim"
setup
cat > Mkfile <<'EOF'
FILES = a.c b.c c.c
EOF
mkV '${FILES:M*.c:R:J,}'
eq "$OUT" "a,b,c" "-V '\${...}' full expression"
mkV 'FILES'
eq "$OUT" "a.c b.c c.c" "-V 'FILES' (no dollar) wraps in \${}"

begin "CLI: -f selects an alternate makefile"
setup
cat > Mkfile <<'EOF'
all:
	@echo from-default
EOF
cat > other.mk <<'EOF'
all:
	@echo from-other
EOF
mkf other.mk
eq "$OUT" "from-other" "-f other.mk"

begin "CLI: -f last occurrence wins"
setup
printf 'all:\n\t@echo one\n' > a.mk
printf 'all:\n\t@echo two\n' > b.mk
OUT=$("$MK" -f a.mk -f b.mk 2>"$WORK/.err"); RC=$?
eq "$OUT" "two" "only the last -f is honored"

begin "CLI: -C changes the working directory"
setup
mkdir sub
cat > sub/Mkfile <<'EOF'
all:
	@echo from-sub
EOF
OUT=$("$MK" -C sub 2>"$WORK/.err"); RC=$?
eq "$OUT" "from-sub" "-C sub"
rc_ok "-C rc"

begin "CLI: -o creates objdir and sets .OBJDIR"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${.OBJDIR}"
EOF
OUT=$("$MK" -o artifacts 2>"$WORK/.err"); RC=$?
rc_ok "-o rc"
file_exists "artifacts" "-o created the directory"
matches "$OUT" "artifacts" "-o is reflected in \${.OBJDIR}"

begin "CLI: -s suppresses command echo (but not recipe output)"
setup
cat > Mkfile <<'EOF'
all:
	echo hello
EOF
OUT=$("$MK" -s 2>"$WORK/.err"); RC=$?
absent "$OUT" "\$ echo hello" "-s: no '[scope] \$ echo' line"
contains "$OUT" "hello" "-s: the recipe output still appears"

begin "CLI: default goal when no target given"
setup
cat > Mkfile <<'EOF'
first:
	@echo first-built
second:
	@echo second-built
EOF
mkrun
eq "$OUT" "first-built" "first target is the default goal"

begin "CLI: .DEFAULT overrides the default goal"
setup
cat > Mkfile <<'EOF'
.DEFAULT: second
first:
	@echo first-built
second:
	@echo second-built
EOF
mkrun
eq "$OUT" "second-built" ".DEFAULT picks second"

begin "CLI: -h prints usage and the help page, exits 1"
setup
cat > Mkfile <<'EOF'
## The greeting target
all:
	@echo hi
EOF
OUT=$("$MK" -h 2>"$WORK/.err"); RC=$?
ERR=$("$MK" -h 2>&1 1>/dev/null)
DOC=$("$MK" -h 2>/dev/null)
rc_is 1 "-h exits 1"
contains "$ERR" "USAGE" "-h prints USAGE on stderr"
contains "$DOC" "The greeting target" "-h lists documented targets on stdout"

begin "CLI: -p dumps parsed state, exits 0"
setup
cat > Mkfile <<'EOF'
VAR = x
all:
	@echo hi
EOF
mkrun -p
rc_ok "-p rc 0"
contains "$OUT" "VAR" "-p shows macros"

begin "CLI: -k keeps going after a failed command"
setup
cat > Mkfile <<'EOF'
broken:
	@false
ok:
	@echo ok-built
all: broken ok
EOF
mkrun -k all
rc_fail "-k: overall failure still reported"
contains "$OUT" "ok-built" "-k: independent target still ran"

begin "CLI: -S stops at the first failure"
setup
cat > Mkfile <<'EOF'
broken:
	@false
ok:
	@echo ok-built
all: broken ok
EOF
mkrun -S all
absent "$OUT" "ok-built" "-S: stops, ok not reached"

begin "CLI: unknown option exits non-zero"
setup
cat > Mkfile <<'EOF'
all:
	@echo hi
EOF
OUT=$("$MK" -Z 2>"$WORK/.err"); RC=$?
rc_fail "unknown option rejected"

finish
