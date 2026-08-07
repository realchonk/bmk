#!/bin/sh
. "$TESTDIR/common.sh"

begin ".FOREIGN + ! exec hook builds an artifact in the subdir"
setup
mkdir dep
cat > Mkfile <<'EOF'
.FOREIGN: dep

all: dep/built

dep?:
	@test -f $&/built || exit 1

dep!:
	@echo "made" > $&/built
EOF
mkrun
rc_ok "foreign build succeeded"
file_exists "dep/built" "the exec hook created the artifact"
eq "$(cat dep/built 2>/dev/null)" "made" "artifact content is correct"

begin "\$@ is the file name and \$& is the subdir name in a hook"
setup
mkdir dep
cat > Mkfile <<'EOF'
.FOREIGN: dep
all: dep/built
dep?:
	@exit 1
dep!:
	@echo "tgt=$@ dir=$&" > $&/built
EOF
mkrun
eq "$(cat dep/built 2>/dev/null)" "tgt=built dir=dep" "\$@ and \$& in a foreign hook"

begin "the ? test hook gates the ! exec hook"
setup
mkdir dep
cat > Mkfile <<'EOF'
.FOREIGN: dep
all: dep/built
runs := 0
dep?:
	@test -f $&/built
dep!:
	@echo built > $&/built
	@echo exec-ran
EOF
mkrun
contains "$OUT" "exec-ran" "first build: test failed, so exec ran"
rm -f dep/.marker 2>/dev/null
: > "$WORK/.second"
mkrun
absent "$OUT" "exec-ran" "second build: test passed, so exec skipped"

begin ".EXPORTS renders exported macros inside foreign hooks"
setup
mkdir dep
cat > Mkfile <<'EOF'
CC = cc
CFLAGS = -O2
.EXPORTS: CC CFLAGS
.FOREIGN: dep
all: dep/built
dep?:
	@exit 1
dep!:
	@echo "exports=[${.EXPORTS}]" > $&/built
EOF
mkrun
eq "$(cat dep/built 2>/dev/null)" "exports=[CFLAGS='-O2' CC='cc']" ".EXPORTS available in hooks"

begin "a bare foreign name declares build ordering"
setup
mkdir liba libb
cat > Mkfile <<'EOF'
.FOREIGN: liba libb
libb: liba
all: libb/stamp
liba!:
	@echo "a done" > liba/stamp
	@echo built-a
libb!:
	@test -f liba/stamp
	@echo "b done" > libb/stamp
	@echo built-b
EOF
mkrun
contains "$OUT" "built-a" "liba was built first"
contains "$OUT" "built-b" "libb built after liba's stamp existed"
file_exists "libb/stamp" "libb's artifact produced"

begin "macros are expanded in the .FOREIGN directory list"
setup
mkdir ext
cat > Mkfile <<'EOF'
SUB = ext
.FOREIGN: ${SUB}
all: ext/built
ext?:
	@exit 1
ext!:
	@echo built > $&/built
EOF
mkrun
file_exists "ext/built" "macro-expanded foreign name descended into"

begin "a path-typed target makes one foreign goal depend on another"
setup
mkdir x
cat > Mkfile <<'EOF'
.FOREIGN: x
x!:
	@echo "built $@"
x/install: x/all
EOF
mkrun x/install
contains "$OUT" "built all" "x/all was built as a prerequisite of x/install"
contains "$OUT" "built install" "x/install was then built"

begin "a shared foreign prerequisite is built only once"
setup
mkdir x
cat > Mkfile <<'EOF'
.FOREIGN: x
x!:
	@echo "built $@"
a: x/all
	@echo built-a
b: x/all
	@echo built-b
top: a b
EOF
mkrun top
eq "$(printf '%s\n' "$OUT" | grep -c 'built all')" "1" "x/all built once across a and b"

finish
