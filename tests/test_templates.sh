#!/bin/sh
. "$TESTDIR/common.sh"

begin ".template/.endt + .expand inlines a block"
setup
cat > Mkfile <<'EOF'
NAME = prog
.template greet
greet-${NAME}:
	@echo hi-${NAME}
.endt
.expand greet
all: greet-prog
EOF
mkrun
eq "$OUT" "hi-prog" "expanded template produced a buildable target"

begin ".endtemplate is a synonym for .endt"
setup
cat > Mkfile <<'EOF'
.template block
made:
	@echo made-it
.endtemplate
.expand block
all: made
EOF
mkrun
eq "$OUT" "made-it" ".endtemplate closes the template"

begin "templates are parameterised by macros at the expand site"
setup
cat > Mkfile <<'EOF'
.template rule
${WHO}-target:
	@echo "name=$@"
.endt
WHO = alice
.expand rule
WHO = bob
.expand rule
EOF
mkrun alice-target
contains "$OUT" "name=alice-target" "first expansion created alice-target"
mkrun bob-target
contains "$OUT" "name=bob-target" "second expansion created bob-target"

begin "a template may contain conditionals"
setup
cat > Mkfile <<'EOF'
.template maybe
.if defined(FLAG)
flagged:
	@echo on
.else
flagged:
	@echo off
.endif
.endt
FLAG = 1
.expand maybe
all: flagged
EOF
mkrun
eq "$OUT" "on" "conditional inside a template is evaluated on expansion"

begin "documentation comments inside a template survive expansion"
setup
cat > Mkfile <<'EOF'
NAME = thing
.template doc
## Build ${NAME}
${NAME}:
	@echo built-${NAME}
.endt
.expand doc
EOF
DOC=$("$MK" -h 2>/dev/null)
contains "$DOC" "Build thing" "doc-comment from the template is shown by -h"

begin "the same template can be expanded more than once"
setup
cat > Mkfile <<'EOF'
.template unit
unit-${N}:
	@echo "built=$@"
.endt
N = one
.expand unit
N = two
.expand unit
EOF
mkrun unit-one
contains "$OUT" "built=unit-one" "first expansion produced unit-one"
mkrun unit-two
contains "$OUT" "built=unit-two" "second expansion produced unit-two"

finish
