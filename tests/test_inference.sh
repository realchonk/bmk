#!/bin/sh
. "$TESTDIR/common.sh"

begin "inference rule builds .out from .txt"
setup
printf 'hello\n' > msg.txt
cat > Mkfile <<'EOF'
all: msg.out

.txt.out:
	cp $< $@
EOF
mkrun
rc_ok "inference build succeeds"
file_exists "msg.out" "the inferred target was created"
eq "$(cat msg.out 2>/dev/null)" "hello" "and its content is the copied source"

begin "inference recipe uses \$< (source) and \$@ (target)"
setup
printf 'data\n' > data.dat
cat > Mkfile <<'EOF'
all: data.bin

.dat.bin:
	echo "src=$< tgt=$@" > $@
EOF
mkrun
eq "$(cat data.bin 2>/dev/null)" "src=data.dat tgt=data.bin" "\$< and \$@ inside an inference rule"

begin "inference chains may be arbitrarily deep"
setup
printf 'start\n' > base.x
cat > Mkfile <<'EOF'
all: base.z

.x.y:
	cp $< $@

.y.z:
	cp $< $@
EOF
mkrun
rc_ok "two-step inference chain"
file_exists "base.z" "final artifact produced through the chain"
eq "$(cat base.z 2>/dev/null)" "start" "content propagated through both steps"

begin "no .SUFFIXES declaration is required"
setup
printf 's\n' > s.l
cat > Mkfile <<'EOF'
all: s.q
.l.q:
	cp $< $@
EOF
mkrun
rc_ok "inference works without .SUFFIXES"
file_exists "s.q" "artifact built"

begin "explicit recipe takes priority over inference"
setup
printf 'from-inference\n' > t.src
cat > Mkfile <<'EOF'
all: t.dst
.src.dst:
	cp $< $@
t.dst:
	echo explicit > $@
EOF
mkrun
eq "$(cat t.dst 2>/dev/null)" "explicit" "explicit rule wins over inference"

begin "inference rule may declare its own prerequisites"
setup
printf 'p\n' > main.p
cat > Mkfile <<'EOF'
all: main.r
.p.r: header
	cat $< header > $@
header:
	echo HEADER > header
EOF
mkrun
rc_ok "inference with extra prereq"
contains "$(cat main.r 2>/dev/null)" "p" "inference source included"

begin ".SUFFIXES: only emits a warning"
setup
cat > Mkfile <<'EOF'
.SUFFIXES:
all:
	@echo still-works
EOF
mkrun
contains "$ERR" "SUFFIXES" ".SUFFIXES produces a warning"
eq "$OUT" "still-works" "build proceeds normally"

finish
