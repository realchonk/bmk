#!/bin/sh
. "$TESTDIR/common.sh"

begin ".SUBDIRS declares a child scope that is descended into"
setup
mkdir sub
cat > sub/Mkfile <<'EOF'
greet:
	@echo hello-from-sub
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: sub
all: sub/greet
	@echo top-done
EOF
mkrun
eq "$OUT" "$(printf 'hello-from-sub\ntop-done')" "descended, then top recipe ran"

begin "subdirectories are parsed lazily"
setup
mkdir broken
cat > broken/Mkfile <<'EOF'
this is not valid make syntax ))))))
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: broken
all:
	@echo top-ok
EOF
mkrun
eq "$OUT" "top-ok" "unused subdir makefile is not parsed"
rc_ok "no error from the unused subdir"

begin "a prerequisite path auto-descends into a declared subdir"
setup
mkdir lib
cat > lib/Mkfile <<'EOF'
lib.a:
	@echo building-lib > lib.a
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: lib
app: lib/lib.a
	@echo app-done
EOF
mkrun app
file_exists "lib/lib.a" "subdir artifact was built"
contains "$OUT" "app-done" "dependent target then ran"

begin ".SUBDIRS:=/target recurses a goal into every subdir"
setup
mkdir a b
cat > a/Mkfile <<'EOF'
clean:
	@echo clean-a
EOF
cat > b/Mkfile <<'EOF'
clean:
	@echo clean-b
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: a b
clean: ${.SUBDIRS:=/clean}
	@echo clean-top
EOF
mkrun clean
contains "$OUT" "clean-a" "recursed into a"
contains "$OUT" "clean-b" "recursed into b"
contains "$OUT" "clean-top" "ran the top clean too"

begin ".SUBDIRS may itself be built from a macro"
setup
mkdir d1 d2
cat > d1/Mkfile <<'EOF'
all:
	@echo d1
EOF
cat > d2/Mkfile <<'EOF'
all:
	@echo d2
EOF
cat > Mkfile <<'EOF'
DIRS = d1 d2
.SUBDIRS: ${DIRS}
all: ${.SUBDIRS}
EOF
mkrun
contains "$OUT" "d1" "macro-expanded subdir list: d1"
contains "$OUT" "d2" "macro-expanded subdir list: d2"

begin "an undeclared subdir prerequisite is treated as a plain file"
setup
mkdir vendor
printf 'vendored\n' > vendor/blob.txt
cat > Mkfile <<'EOF'
use: vendor/blob.txt
	@echo "have-vendored-file"
EOF
mkrun use
rc_ok "undeclared subdir file is stat-checked, not built"
contains "$OUT" "have-vendored-file" "the build proceeded using the existing file"

begin "a path-typed target makes one subdir goal depend on another"
setup
mkdir sub
cat > sub/Mkfile <<'EOF'
all:
	@echo sub-all
install:
	@echo sub-install
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: sub
sub/install: sub/all
EOF
mkrun sub/install
contains "$OUT" "sub-all" "sub/all was built as a prerequisite of sub/install"
contains "$OUT" "sub-install" "sub/install was then built"

finish
