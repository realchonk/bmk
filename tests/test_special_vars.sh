#!/bin/sh
. "$TESTDIR/common.sh"

begin ".TARGET / \$@ is the target being built"
setup
cat > Mkfile <<'EOF'
mytarget:
	@echo "T=${.TARGET} short=$@"
EOF
mkrun mytarget
eq "$OUT" "T=mytarget short=mytarget" ".TARGET == \$@"

begin ".ALLSRC / \$^ lists all prerequisites"
setup
: > a
: > b
: > c
cat > Mkfile <<'EOF'
out: a b c
	@echo "ALL=${.ALLSRC} short=$^"
EOF
mkrun out
contains "$OUT" "a b c" ".ALLSRC and \$^ both carry the prerequisite list"

begin ".IMPSRC / \$< is the first prerequisite"
setup
: > first
: > second
cat > Mkfile <<'EOF'
out: first second
	@echo "IMP=${.IMPSRC} short=$<"
EOF
mkrun out
eq "$OUT" "IMP=first short=first" ".IMPSRC == \$<"

begin ".SCOPE and .SCOPE:T at the top level"
setup
cat > Mkfile <<'EOF'
all:
	@echo "SCOPE=${.SCOPE} T=${.SCOPE:T} amp=$&"
EOF
mkrun
eq "$OUT" "SCOPE=. T=. amp=." "top-level scope is '.'"

begin ".TOPDIR at the top level is '.'"
setup
cat > Mkfile <<'EOF'
all:
	@echo "TOP=${.TOPDIR}"
EOF
mkrun
eq "$OUT" "TOP=." ".TOPDIR is '.' at the root"

begin ".SCOPE / .TOPDIR inside a subdirectory"
setup
mkdir -p sub
cat > sub/Mkfile <<'EOF'
all:
	@echo "SCOPE=${.SCOPE} T=${.SCOPE:T} TOP=${.TOPDIR}"
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: sub
EOF
mkrun sub/all
eq "$OUT" "SCOPE=./sub T=sub TOP=./.." "subdir scope and topdir are relative"

begin ".OBJDIR defaults to the current directory"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${.OBJDIR}"
EOF
mkrun
eq "$OUT" "." ".OBJDIR defaults to '.'"

begin ".SUBDIRS lists declared subdirectories"
setup
mkdir alpha beta gamma
cat > Mkfile <<'EOF'
.SUBDIRS: alpha beta gamma
all:
	@echo "${.SUBDIRS}"
EOF
mkrun
contains "$OUT" "alpha" ".SUBDIRS carries alpha"
contains "$OUT" "beta" ".SUBDIRS carries beta"
contains "$OUT" "gamma" ".SUBDIRS carries gamma"

begin ".EXPORTS renders NAME='value' pairs"
setup
cat > Mkfile <<'EOF'
CC = cc
CFLAGS = -O2 -g
PREFIX = /usr/local
.EXPORTS: CC CFLAGS
all:
	@echo "${.EXPORTS}"
EOF
mkrun
eq "$OUT" "CFLAGS='-O2 -g' CC='cc'" ".EXPORTS formats exports for foreign tools"

begin "MAKE / .MAKE is the invocation name"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${MAKE}"
EOF
mkrun
# MAKE is argv[0] of mk
matches "$OUT" "mk" "MAKE contains the binary name"

begin "MAKEFLAGS / .MAKEFLAGS is populated"
setup
cat > Mkfile <<'EOF'
all:
	@echo "[${MAKEFLAGS}]"
EOF
OUT=$("$MK" -s -v 2>"$WORK/.err"); RC=$?
contains "$OUT" "-s" "MAKEFLAGS records -s"
contains "$OUT" "-v" "MAKEFLAGS records -v"

begin "\$\$ is a literal dollar sign"
setup
cat > Mkfile <<'EOF'
all:
	@echo 'cost=$$5'
EOF
mkrun
eq "$OUT" "cost=\$5" "\$\$ survives expansion"

begin ".MAKEFILES lists makefiles on the path from root"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${.MAKEFILES}"
EOF
mkrun
contains "$OUT" "Mkfile" ".MAKEFILES includes Mkfile"

begin "SHELL defaults to /bin/sh"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${SHELL}"
EOF
mkrun
eq "$OUT" "sh" "SHELL is 'sh' by default"

begin ".IMPSRC:T / \$* is the basename of the implied source"
setup
mkdir -p src
: > src/main.c
cat > Mkfile <<'EOF'
prog: src/main.c
	@echo "STAR=${.IMPSRC:T}"
EOF
mkrun prog
eq "$OUT" "STAR=main.c" ".IMPSRC:T strips the directory"

finish
