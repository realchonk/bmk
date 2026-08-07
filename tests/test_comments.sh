#!/bin/sh
. "$TESTDIR/common.sh"

begin "a single-# comment is ignored"
setup
cat > Mkfile <<'EOF'
# this is a comment
VAL = real
# another comment
all:
	@echo "${VAL}"
EOF
mkrun
eq "$OUT" "real" "comment lines did not interfere"

begin "a comment may trail a dependency line"
setup
cat > Mkfile <<'EOF'
all: dep   # trailing comment
	@echo built
dep:
	@echo dep-built
EOF
mkrun
contains "$OUT" "built" "trailing comment stripped from the dep line"

begin "## documents the next macro definition"
setup
cat > Mkfile <<'EOF'
## The install prefix
PREFIX = /usr/local
EOF
DOC=$("$MK" -h 2>/dev/null)
contains "$DOC" "PREFIX" "-h lists the documented macro"
contains "$DOC" "The install prefix" "-h shows the macro's help text"

begin "## documents the next target/rule"
setup
cat > Mkfile <<'EOF'
## Build everything
all:
	@echo hi
EOF
DOC=$("$MK" -h 2>/dev/null)
contains "$DOC" "all" "-h lists the documented target"
contains "$DOC" "Build everything" "-h shows the target's help text"

begin "macro references in a doc-comment are expanded"
setup
cat > Mkfile <<'EOF'
NAME = widget
## Build the ${NAME} program
all:
	@echo built
EOF
DOC=$("$MK" -h 2>/dev/null)
contains "$DOC" "Build the widget program" "\${NAME} was expanded in the doc-comment"

begin "a # inside a recipe is passed to the shell"
setup
cat > Mkfile <<'EOF'
hashy:
	@echo before # trailing shell comment
EOF
mkrun hashy
eq "$OUT" "before" "the shell saw and honored the # comment"

begin "a blank line does nothing"
setup
cat > Mkfile <<'EOF'

VAL = kept


all:
	@echo "${VAL}"
EOF
mkrun
eq "$OUT" "kept" "blank lines are tolerated"

begin "consecutive ## attach only the last one as help"
setup
cat > Mkfile <<'EOF'
## first line
## second line
all:
	@echo x
EOF
DOC=$("$MK" -h 2>/dev/null)
contains "$DOC" "second line" "the doc comment immediately above the rule wins"

finish
