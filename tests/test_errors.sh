#!/bin/sh
. "$TESTDIR/common.sh"

begin "no makefile at all is fatal"
setup
rm -f Mkfile
mkrun
rc_fail "missing makefile exits non-zero"
contains "$ERR" "makefile" "error mentions the makefile"

begin "a prerequisite that cannot be found is fatal"
setup
cat > Mkfile <<'EOF'
out: nonexistent.source
	@echo built
EOF
mkrun out
rc_fail "unbuildable prereq exits non-zero"
contains "$ERR" "no such file" "error reports the missing file"

begin "a target with no recipe and no source file is fatal"
setup
cat > Mkfile <<'EOF'
all: phantom
EOF
mkrun all
rc_fail "no way to build 'phantom'"
contains "$ERR" "no such file" "error reports the missing file"

begin "an invalid line produces a warning but keeps going"
setup
cat > Mkfile <<'EOF'
this has no colon and no equals
all:
	@echo recovered
EOF
mkrun
rc_ok "invalid line is non-fatal"
contains "$ERR" "invalid line" "warning emitted on stderr"
contains "$OUT" "recovered" "the build still completed"

begin ".POSIX: emits a compatibility warning"
setup
cat > Mkfile <<'EOF'
.POSIX:
all:
	@echo ok
EOF
mkrun
rc_ok ".POSIX is accepted"
contains "$ERR" "POSIX" "POSIX warning printed"

begin ".SUFFIXES: emits a compatibility warning"
setup
cat > Mkfile <<'EOF'
.SUFFIXES:
all:
	@echo ok
EOF
mkrun
rc_ok ".SUFFIXES is accepted"
contains "$ERR" "SUFFIXES" "SUFFIXES warning printed"

begin "a failing recipe command is fatal by default"
setup
cat > Mkfile <<'EOF'
fail:
	false
	@echo unreachable
EOF
mkrun fail
rc_fail "failing recipe aborts"
absent "$OUT" "unreachable" "later commands skipped"

begin "errors report the scope and the failing command"
setup
cat > Mkfile <<'EOF'
all:
	@echo line-2-ok
	@false
	@echo line-4
EOF
mkrun
rc_fail "the failing line aborted the build"
contains "$ERR" "command failed" "error reports the failed command"

finish
