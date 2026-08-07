# shellcheck shell=sh
# Common helpers for the bmk test suite.
# Sourced by every test_*.sh; run.sh sets TESTDIR before invoking them.

NTOTAL=0
NPASSED=0
NFAILED=0
WORK=""
MK="${MK:-}"

if [ -z "$MK" ]; then
	echo "common.sh: MK is not set (run via run.sh)" >&2
	exit 99
fi

# Print a section header. Counters accumulate across the whole file so that a
# failure in any section is reflected in the final tally.
begin() {
	printf '\n== %s ==\n' "$1"
}

# Create a fresh working directory and enter it. Every test runs in its own
# throwaway tree so makefiles and generated files never leak between cases.
setup() {
	WORK=$(mktemp -d 2>/dev/null || mktemp -d -t bmk)
	OLDDIR=$(pwd)
	cd "$WORK" || exit 99
}

teardown() {
	[ -n "$WORK" ] || return 0
	cd "$OLDDIR" 2>/dev/null || true
	rm -rf "$WORK"
	WORK=""
}

# mkrun [args...] -- run $MK in the current work dir; capture OUT, ERR, RC.
mkrun() {
	OUT=$("$MK" "$@" 2>"$WORK/.err")
	RC=$?
	ERR=$(cat "$WORK/.err")
}

# mkf FILE [args...] -- run $MK -f FILE.
mkf() {
	_mf_file=$1
	shift
	OUT=$("$MK" -f "$_mf_file" "$@" 2>"$WORK/.err")
	RC=$?
	ERR=$(cat "$WORK/.err")
}

# Run $MK -V and capture into OUT (no makefile needed if value is literal).
mkV() {
	OUT=$("$MK" -V "$@" 2>"$WORK/.err")
	RC=$?
	ERR=$(cat "$WORK/.err")
}

# --- internal recorders -----------------------------------------------
_rec_ok() {
	NTOTAL=$((NTOTAL + 1))
	NPASSED=$((NPASSED + 1))
	printf '  ok   %s\n' "$1"
}
_rec_bad() {
	NTOTAL=$((NTOTAL + 1))
	NFAILED=$((NFAILED + 1))
	printf '  FAIL %s\n' "$1"
	shift
	while [ $# -gt 0 ]; do
		printf '       %s\n' "$1"
		shift
	done
}

# --- assertions -------------------------------------------------------
# eq GOT WANT DESC
eq() {
	if [ "$1" = "$2" ]; then
		_rec_ok "$3"
	else
		_rec_bad "$3" "got : [$1]" "want: [$2]"
	fi
}

# neq GOT UNWANT DESC
neq() {
	if [ "$1" != "$2" ]; then
		_rec_ok "$3"
	else
		_rec_bad "$3" "got: [$1]" "expected to differ from: [$2]"
	fi
}

# rc_is WANT DESC  -- check $RC
rc_is() {
	eq "$RC" "$1" "$2"
}
rc_ok() {
	eq "$RC" "0" "$1"
}
rc_fail() {
	NTOTAL=$((NTOTAL + 1))
	if [ "$RC" -ne 0 ]; then
		NPASSED=$((NPASSED + 1))
		printf '  ok   %s (rc=%d)\n' "$1" "$RC"
	else
		NFAILED=$((NFAILED + 1))
		printf '  FAIL %s (expected non-zero exit)\n' "$1"
	fi
}

# contains HAYSTACK NEEDLE DESC
contains() {
	case "$1" in
		*"$2"*) _rec_ok "$3" ;;
		*) _rec_bad "$3" "missing [$2]" "in   [$1]" ;;
	esac
}

# absent HAYSTACK NEEDLE DESC
absent() {
	case "$1" in
		*"$2"*) _rec_bad "$3" "unexpected [$2]" "in   [$1]" ;;
		*) _rec_ok "$3" ;;
	esac
}

# matches STRING REGEX DESC  (extended regex via grep -E)
matches() {
	if printf '%s\n' "$1" | grep -Eq "$2"; then
		_rec_ok "$3"
	else
		_rec_bad "$3" "[$1] does not match /$2/"
	fi
}

# file_exists PATH DESC
file_exists() {
	if [ -e "$1" ]; then
		_rec_ok "$2"
	else
		_rec_bad "$2" "missing file: $1"
	fi
}

# Print the per-file summary and exit with the right status.
finish() {
	teardown
	if [ "$NFAILED" -eq 0 ]; then
		printf 'RESULT %d passed\n' "$NTOTAL"
		exit 0
	else
		printf 'RESULT %d/%d FAILED\n' "$NFAILED" "$NTOTAL"
		exit 1
	fi
}
