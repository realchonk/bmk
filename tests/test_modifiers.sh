#!/bin/sh
. "$TESTDIR/common.sh"

# Most modifier checks use -V on a single macro for a clean one-line result.
probe() {
	cat > Mkfile <<EOF
S = ${SAMPLE}
EOF
	mkV "\${S$1}"
}

begin "modifier :U uppercases every word"
setup
SAMPLE="one TWO three Four"
probe ":U"
eq "$OUT" "ONE TWO THREE FOUR" ":U"

begin "modifier :L lowercases every word"
setup
SAMPLE="ONE Two tHREE"
probe ":L"
eq "$OUT" "one two three" ":L"

begin "modifier :T takes the basename of each word"
setup
SAMPLE="a/b/c.c d/e.f x/y/g"
probe ":T"
eq "$OUT" "c.c e.f g" ":T"

begin "modifier :H takes the dirname of each word"
setup
SAMPLE="a/b/c.c d/e.f x/y/g"
probe ":H"
eq "$OUT" "a/b d x/y" ":H"

begin "modifier :E keeps only the suffix"
setup
SAMPLE="a/b/c.c d/e.f x/y/g"
probe ":E"
eq "$OUT" ".c .f" ":E (g has no suffix -> dropped)"

begin "modifier :R strips the suffix"
setup
SAMPLE="a/b/c.c d/e.f x/y/g"
probe ":R"
eq "$OUT" "a/b/c d/e x/y/g" ":R"

begin "modifier :M retains matching words (glob)"
setup
SAMPLE="main.c lex.l parse.y gen.S"
probe ":M*.c"
eq "$OUT" "main.c" ":M*.c"

begin "modifier :N drops matching words (negation of :M)"
setup
SAMPLE="main.c lex.l parse.y gen.S"
probe ":N*.c"
eq "$OUT" "lex.l parse.y gen.S" ":N*.c"

begin "modifier :J joins all words with a separator"
setup
SAMPLE="a b c d"
probe ":J,"
eq "$OUT" "a,b,c,d" ":J,"
probe ":J - "
eq "$OUT" "a - b - c - d" ":J with multi-char separator"

begin "modifier :old=new substitutes a suffix"
setup
SAMPLE="main.c lex.l parse.y"
probe ":.c=.o"
eq "$OUT" "main.o lex.l parse.y" ":.c=.o only touches .c words"

begin "modifier :old=new with empty old appends new to each word"
setup
SAMPLE="foo bar"
probe ":=/x"
eq "$OUT" "foo/x bar/x" ":=/x appends to every word"

begin "modifiers can be chained"
setup
SAMPLE="main.c lex.l parse.y"
probe ":M*.c:.c=.o"
eq "$OUT" "main.o" ":M*.c then :.c=.o"
probe ":T:U"
eq "$OUT" "MAIN.C LEX.L PARSE.Y" ":T (no dir parts) then :U uppercases"

begin ":F returns the word unchanged when no file is found"
setup
mkdir src
printf 'int main(){return 0;}\n' > src/a.c
printf 'int x;\n' > src/b.c
cat > Mkfile <<'EOF'
SRCS = a.c b.c
all:
	@echo "${SRCS:F}"
EOF
# a.c/b.c do not exist in cwd (only in src/), so :F leaves them as-is
mkrun
eq "$OUT" "a.c b.c" ":F passes through words it cannot resolve"

begin ":F finds files that do exist"
setup
printf 'int main(){return 0;}\n' > a.c
printf 'int x;\n' > b.c
cat > Mkfile <<'EOF'
SRCS = a.c b.c
all:
	@echo "${SRCS:F}"
EOF
mkrun
eq "$OUT" "a.c b.c" ":F resolves existing files"

finish
