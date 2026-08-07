#!/bin/sh
. "$TESTDIR/common.sh"

begin "building creates the target from its source"
setup
printf 'original\n' > data.in
cat > Mkfile <<'EOF'
data.out: data.in
	cp $< $@
EOF
mkrun data.out
rc_ok "build succeeded"
file_exists "data.out" "target created"
eq "$(cat data.out)" "original" "content copied"

begin "an up-to-date target is not rebuilt"
setup
printf 'v1\n' > data.in
cat > Mkfile <<'EOF'
data.out: data.in
	cp $< $@
	echo built >> build.log
EOF
mkrun data.out
mkrun data.out
# Two builds, but the recipe should only have run once.
eq "$(wc -l < build.log 2>/dev/null | tr -d ' ')" "1" "recipe ran exactly once across two builds"

begin "touching a source forces a rebuild"
setup
printf 'v1\n' > data.in
cat > Mkfile <<'EOF'
data.out: data.in
	cp $< $@
	echo built >> build.log
EOF
mkrun data.out
mkrun data.out
sleep 1
touch data.in
mkrun data.out
eq "$(wc -l < build.log 2>/dev/null | tr -d ' ')" "2" "recipe ran again after the source was touched"

begin "the newest of several sources drives the rebuild"
setup
printf 'a\n' > a.src
printf 'b\n' > b.src
cat > Mkfile <<'EOF'
combined: a.src b.src
	cat a.src b.src > combined
	echo built >> build.log
EOF
mkrun combined
mkrun combined
sleep 1
touch b.src
mkrun combined
eq "$(wc -l < build.log 2>/dev/null | tr -d ' ')" "2" "newer second source triggered a rebuild"

begin "a target with no source file is rebuilt every time"
setup
cat > Mkfile <<'EOF'
phony:
	echo run >> phony.log
EOF
mkrun phony
mkrun phony
eq "$(wc -l < phony.log 2>/dev/null | tr -d ' ')" "2" "non-file target rebuilds on each invocation"

begin "a chain of prerequisites builds in order"
setup
printf '1\n' > step1
cat > Mkfile <<'EOF'
final: step2
	cat step2 > final
step2: step1
	cat step1 > step2
EOF
mkrun final
rc_ok "chain built"
file_exists "step2" "intermediate produced"
file_exists "final" "final produced"

begin "building a single named target does not build unrelated ones"
setup
cat > Mkfile <<'EOF'
wanted:
	@echo wanted-built
other:
	@echo other-built
EOF
mkf Mkfile wanted
contains "$OUT" "wanted-built" "named target built"
absent "$OUT" "other-built" "unrelated target was not built"

begin ".DEFAULT selects the goal for bare 'mk'"
setup
cat > Mkfile <<'EOF'
.DEFAULT: realgoal
realgoal:
	@echo realgoal-built
decoy:
	@echo decoy-built
EOF
mkrun
eq "$OUT" "realgoal-built" ".DEFAULT wins over the first target"

begin "a shared prerequisite is built only once (diamond)"
setup
cat > Mkfile <<'EOF'
shared:
	@echo mark >> run.log
a: shared
	@echo mark >> run.log
b: shared
	@echo mark >> run.log
top: a b
EOF
mkrun top
eq "$(grep -c mark run.log 2>/dev/null)" "3" "shared + a + b each ran once (shared not rebuilt)"

begin "a duplicate prerequisite is built only once"
setup
cat > Mkfile <<'EOF'
dep:
	@echo mark >> run.log
top: dep dep
EOF
mkrun top
eq "$(grep -c mark run.log 2>/dev/null)" "1" "duplicate prerequisite built once"

finish
