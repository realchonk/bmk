#!/bin/sh
. "$TESTDIR/common.sh"

begin "include pulls in another makefile"
setup
cat > rules.mk <<'EOF'
INCLUDED = yes-from-include
EOF
cat > Mkfile <<'EOF'
include rules.mk
all:
	@echo "${INCLUDED}"
EOF
mkrun
eq "$OUT" "yes-from-include" "macro from included file is visible"

begin "macros defined before include are visible inside it"
setup
cat > dep.mk <<'EOF'
RESULT = ${BASE}/lib
EOF
cat > Mkfile <<'EOF'
BASE = /usr
include dep.mk
all:
	@echo "${RESULT}"
EOF
mkrun
eq "$OUT" "/usr/lib" "included file sees the caller's macros"

begin "-include tolerates a missing file"
setup
cat > Mkfile <<'EOF'
-include does-not-exist.mk
all:
	@echo ok
EOF
mkrun
eq "$OUT" "ok" "-include is silent when the file is absent"

begin "sinclude is a synonym for -include"
setup
cat > Mkfile <<'EOF'
sinclude does-not-exist.mk
all:
	@echo ok
EOF
mkrun
eq "$OUT" "ok" "sinclude tolerates a missing file"

begin "a missing 'include' is currently tolerated (like -include)"
setup
cat > Mkfile <<'EOF'
include definitely-missing.mk
all:
	@echo ran-anyway
EOF
mkrun
rc_ok "missing include does not abort (NOTE: differs from the man page, which treats 'include' as required)"
eq "$OUT" "ran-anyway" "the build continued past the missing include"

begin "macros can be expanded in the include path"
setup
mkdir parts
cat > parts/extra.mk <<'EOF'
EXTRA = loaded
EOF
cat > Mkfile <<'EOF'
PART = extra
include parts/${PART}.mk
all:
	@echo "${EXTRA}"
EOF
mkrun
eq "$OUT" "loaded" "include path was macro-expanded"

begin "nested includes resolve relative to the including scope"
setup
cat > inner.mk <<'EOF'
DEEP = nested-value
EOF
cat > outer.mk <<'EOF'
include inner.mk
EOF
cat > Mkfile <<'EOF'
include outer.mk
all:
	@echo "${DEEP}"
EOF
mkrun
eq "$OUT" "nested-value" "include depth chains correctly"

finish
