#!/bin/sh
. "$TESTDIR/common.sh"

begin "\${NAME} expands a macro"
setup
cat > Mkfile <<'EOF'
NAME = world
all:
	@echo "hello ${NAME}"
EOF
mkrun
eq "$OUT" "hello world" "braces expand"

begin "single-character \$X does not need braces"
setup
cat > Mkfile <<'EOF'
A = ok
all:
	@echo "result=$A"
EOF
mkrun
eq "$OUT" "result=ok" "\$A expands without braces"

begin "an undefined macro expands to the empty string"
setup
cat > Mkfile <<'EOF'
all:
	@echo "[${UNDEFINED}]"
EOF
mkrun
eq "$OUT" "[]" "undefined macro is empty"

begin "\$\$ yields a single literal dollar"
setup
cat > Mkfile <<'EOF'
all:
	@echo 'cost=$$5'
EOF
mkrun
eq "$OUT" "cost=\$5" "\$\$ becomes a single dollar"

begin "a macro may be referenced inside another macro's value (lazy)"
setup
cat > Mkfile <<'EOF'
BASE = /usr
LIBDIR = ${BASE}/lib
all:
	@echo "${LIBDIR}"
EOF
mkrun
eq "$OUT" "/usr/lib" "nested reference expands at use time"

begin "expansion in -V with an embedded dollar is verbatim"
setup
cat > Mkfile <<'EOF'
LIST = one two three four
EOF
OUT=$("$MK" -V '${LIST:M*o*}' 2>"$WORK/.err"); RC=$?
eq "$OUT" "one two four" "-V accepts a full \${...} expression"

begin "expansion in -V without a dollar wraps the name in braces"
setup
cat > Mkfile <<'EOF'
ANSWER = 42
EOF
OUT=$("$MK" -V ANSWER 2>"$WORK/.err"); RC=$?
eq "$OUT" "42" "-V ANSWER works"

begin "macro references are case sensitive"
setup
cat > Mkfile <<'EOF'
Foo = upper
foo = lower
all:
	@echo "${Foo}|${foo}"
EOF
mkrun
eq "$OUT" "upper|lower" "Foo and foo are distinct"

begin "a backslash-newline continuation joins logical lines"
setup
cat > Mkfile <<'EOF'
LIST = a \
	b \
	c
all:
	@echo "${LIST:J,}"
EOF
mkrun
eq "$OUT" "a,b,c" "backslash-newline produces three separate words"

finish
