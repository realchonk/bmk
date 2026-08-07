#!/bin/sh
. "$TESTDIR/common.sh"

# Helper: build a makefile whose body runs '.if <expr>' and echoes YES/NO.
checkif() {
	cat > Mkfile <<EOF
.if $1
R=YES
.else
R=NO
.endif
all:
	@echo "\${R}"
EOF
	mkrun
}

begin ".if with a truthy non-empty string"
setup
checkif '"yes"'
eq "$OUT" "YES" "non-empty string is true"

begin ".if with an empty string is false"
setup
checkif '""'
eq "$OUT" "NO" "empty string is false"

begin "defined(name)"
setup
cat > Mkfile <<'EOF'
FOO = 1
.if defined(FOO)
R=have
.else
R=miss
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "have" "defined() sees FOO"

begin "!defined(name)"
setup
checkif '!defined(NOSUCHMACRO)'
eq "$OUT" "YES" "!defined on an absent macro"

begin "string equality =="
setup
checkif '"abc" == "abc"'
eq "$OUT" "YES" "== matches"

begin "string inequality !="
setup
checkif '"abc" != "abd"'
eq "$OUT" "YES" "!= differs"

begin "numeric comparison with quoted operands (integer)"
setup
checkif '"10" > "9"'
eq "$OUT" "YES" "'10' > '9' compares as integers, not lexicographically"

begin "all comparison operators"
setup
for e in '"2" > "1"' '"1" < "2"' '"2" >= "2"' '"1" <= "1"' '"1" == "1"' '"1" != "2"'; do
	checkif "$e"
	eq "$OUT" "YES" "$e is true"
done

begin "logical && and ||"
setup
cat > Mkfile <<'EOF'
A=1
.if defined(A) || defined(B)
R=YES
.else
R=NO
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "YES" "|| true when one side holds"
cat > Mkfile <<'EOF'
A=1
.if defined(A) && defined(A)
R=YES
.else
R=NO
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "YES" "&& true when both hold"

begin "|| is false when neither side holds"
setup
checkif 'defined(A) || defined(B)'
eq "$OUT" "NO" "|| false when both operands undefined"

begin "parenthesised sub-expressions"
setup
checkif '("a" == "a") && ("b" == "b")'
eq "$OUT" "YES" "(a==a) && (b==b)"

begin "nested parentheses and grouping"
setup
checkif '(defined(X) || defined(Y)) && "z"'
eq "$OUT" "NO" "(false || false) && true => false"
checkif '!(defined(X))'
eq "$OUT" "YES" "!(false) => true"

begin "backtick command substitution"
setup
checkif '`echo 1` == "1"'
eq "$OUT" "YES" "backtick output compares equal"
checkif '`printf yes`'
eq "$OUT" "YES" "backtick truthy"

begin "target(name) predicate"
setup
cat > Mkfile <<'EOF'
all:
	@echo "${R}"
.if target(all)
R=HASALL
.else
R=NOALL
.endif
EOF
mkrun
eq "$OUT" "HASALL" "target(all) is defined once the rule has been seen"

begin ".if/.elif/.else chain"
setup
cat > Mkfile <<'EOF'
MODE = release
.if "${MODE}" == "debug"
R=d
.elif "${MODE}" == "release"
R=r
.else
R=o
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "r" ".elif selects the right branch"

begin "macro references inside quoted strings are expanded"
setup
cat > Mkfile <<'EOF'
OS = Linux
.if "${OS}" == "Linux"
R=linux
.else
R=other
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "linux" "\${OS} expanded inside the quoted atom"

begin "conditionally-undefined blocks are not parsed"
setup
cat > Mkfile <<'EOF'
.if defined(NEVER)
BROKEN SYNTAX HERE }}}}
.else
R=ok
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "ok" "the false branch is skipped entirely"

begin "nested .if up to several levels"
setup
cat > Mkfile <<'EOF'
A=1
B=1
.if defined(A)
.if defined(B)
R=both
.else
R=onlyA
.endif
.else
R=neither
.endif
all:
	@echo "${R}"
EOF
mkrun
eq "$OUT" "both" "two-level nesting works"

finish
