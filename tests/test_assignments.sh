#!/bin/sh
. "$TESTDIR/common.sh"

begin "operator = is lazy (expanded on each reference)"
setup
cat > Mkfile <<'EOF'
X = first
Y = ${X}
X = second
all:
	@echo "${Y}"
EOF
mkrun
eq "$OUT" "second" "lazy = picks up later X"

begin "operator := is immediate (freezes the value)"
setup
cat > Mkfile <<'EOF'
X = first
Y := ${X}
X = second
all:
	@echo "${Y}"
EOF
mkrun
eq "$OUT" "first" ":= freezes Y at the time of assignment"

begin "operator ::= is equivalent to :="
setup
cat > Mkfile <<'EOF'
X = first
Y ::= ${X}
X = second
all:
	@echo "${Y}"
EOF
mkrun
eq "$OUT" "first" "::= freezes like :="

begin "operator += appends with a single separating space"
setup
cat > Mkfile <<'EOF'
A = one
A += two
A += three
all:
	@echo "[${A}]"
EOF
mkrun
eq "$OUT" "[one two three]" "+= joins words with spaces"

begin "operator += works on a previously-empty macro"
setup
cat > Mkfile <<'EOF'
A +=
A += x
all:
	@echo "[${A}]"
EOF
mkrun
# leading space is acceptable; just must contain x
contains "$OUT" "x" "+= on empty still stores the value"

begin "operator = overrides a previous value"
setup
cat > Mkfile <<'EOF'
A = old
A = new
all:
	@echo "${A}"
EOF
mkrun
eq "$OUT" "new" "= replaces"

begin "operator ?= does not override an existing value"
setup
cat > Mkfile <<'EOF'
A = kept
A ?= replaced
all:
	@echo "${A}"
EOF
mkrun
eq "$OUT" "kept" "?= respects an existing definition"

begin "operator ?= assigns when the macro is undefined"
setup
cat > Mkfile <<'EOF'
A ?= defaulted
all:
	@echo "${A}"
EOF
mkrun
eq "$OUT" "defaulted" "?= sets a brand-new macro"

begin "operator ??= reads the environment"
setup
cat > Mkfile <<'EOF'
FOO ??= frommake
all:
	@echo "${FOO}"
EOF
OUT=$(FOO=fromenv "$MK" 2>"$WORK/.err"); RC=$?
eq "$OUT" "fromenv" "??= keeps the environment value"
mkrun
eq "$OUT" "frommake" "??= falls back to the makefile value when env is unset"

begin "operator != captures shell command output"
setup
cat > Mkfile <<'EOF'
REV != echo abc | rev
HOST != printf 'h-%s' host
all:
	@echo "${REV}|${HOST}"
EOF
mkrun
eq "$OUT" "cba|h-host" "!= runs the command and stores stdout"

begin "whitespace around the value is stripped"
setup
cat > Mkfile <<'EOF'
A =    spaced
B :=   spaced
C = kept
all:
	@echo "[${A}][${B}][${C}]"
EOF
mkrun
eq "$OUT" "[spaced][spaced][kept]" "leading/trailing whitespace trimmed"

begin "command-line VAR=val assignments are visible"
setup
cat > Mkfile <<'EOF'
all:
	@echo "CC=${CC}"
EOF
OUT=$("$MK" CC=clang 2>"$WORK/.err"); RC=$?
eq "$OUT" "CC=clang" "command-line assignment reaches the recipe"

begin "macro names may contain dots and underscores"
setup
cat > Mkfile <<'EOF'
MY_VAR = a
MY.VAR = b
all:
	@echo "${MY_VAR}${MY.VAR}"
EOF
mkrun
eq "$OUT" "ab" "names allow _ and ."

finish
