#!/bin/sh
. "$TESTDIR/common.sh"

begin "an explicit target echoes its recipe as [name] \$ cmd"
setup
cat > Mkfile <<'EOF'
foo:
	echo running-foo
EOF
mkrun foo
contains "$OUT" "[foo] \$ echo running-foo" "echo line names the target"

begin "the default goal (no target arg) echoes as [] \$ cmd"
setup
cat > Mkfile <<'EOF'
def:
	echo running-def
EOF
mkrun
contains "$OUT" "[] \$ echo running-def" "default goal has an empty scope tag"

begin "@ prefix suppresses the echo line"
setup
cat > Mkfile <<'EOF'
quiet:
	@echo silent-but-runs
EOF
mkrun quiet
absent "$OUT" "\$ echo" "no echo line for @ command"
contains "$OUT" "silent-but-runs" "the @ command still executed"

begin "- prefix ignores a failing command"
setup
cat > Mkfile <<'EOF'
forgiving:
	-false
	@echo continued-anyway
EOF
mkrun forgiving
rc_ok "- prefix keeps the build green"
contains "$OUT" "continued-anyway" "execution continued past the failure"

begin "a failing command without - aborts the build"
setup
cat > Mkfile <<'EOF'
strict:
	false
	@echo should-not-reach
EOF
mkrun strict
rc_fail "failing command is fatal"
absent "$OUT" "should-not-reach" "later commands did not run"

begin "each command line runs in a fresh shell"
setup
cat > Mkfile <<'EOF'
twostep:
	cd does-not-exist-1 || true
	@echo "PWD=$$(pwd)"
EOF
mkrun twostep
rc_ok "first line failing-cd was tolerated by '|| true'"
contains "$OUT" "PWD=" "second line is a separate shell (cd did not persist)"

begin "without -v the raw (unexpanded) command is echoed"
setup
cat > Mkfile <<'EOF'
X = world
greet:
	echo hi-${X}
EOF
mkrun greet
contains "$OUT" "echo hi-\${X}" "raw command text shown by default"
absent "$OUT" "echo hi-world" "expansion not shown without -v"

begin "with -v the expanded command is echoed"
setup
cat > Mkfile <<'EOF'
X = world
greet:
	echo hi-${X}
EOF
OUT=$("$MK" -v greet 2>"$WORK/.err"); RC=$?
contains "$OUT" "echo hi-world" "-v shows the expanded command"

begin "a hash inside a recipe is passed to the shell as a comment"
setup
cat > Mkfile <<'EOF'
hashy:
	@echo before # this is a shell comment
EOF
mkrun hashy
eq "$OUT" "before" "the hash did not terminate the echoed/printed text early"

begin "recipes for a target in a subdirectory echo [sub/name]"
setup
mkdir sub
cat > sub/Mkfile <<'EOF'
job:
	echo subjob
EOF
cat > Mkfile <<'EOF'
.SUBDIRS: sub
EOF
mkrun sub/job
contains "$OUT" "[sub/job] \$ echo subjob" "subdir recipe names the scope path"

finish
