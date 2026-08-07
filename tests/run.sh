#!/bin/sh
# Test runner for bmk.
#
# Runs every tests/test_*.sh in its own shell and reports a summary.
# Exit status is non-zero if any assertion failed.
#
# Usage:
#   sh tests/run.sh            # use ./mk
#   MK=./mk sh tests/run.sh    # pick the binary
#   MK=/usr/local/bin/mk sh tests/run.sh

set -u

TESTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(dirname -- "$TESTDIR")

# Locate the mk binary.
if [ -z "${MK:-}" ]; then
	if [ -x "$ROOT/mk" ]; then
		MK="$ROOT/mk"
	elif [ -x ./mk ]; then
		MK=./mk
	else
		echo "run.sh: no 'mk' binary found." >&2
		echo "  build it first with:  ./configure && make" >&2
		echo "  or point at one:     MK=/path/to/mk sh tests/run.sh" >&2
		exit 2
	fi
fi

export MK TESTDIR

echo "bmk test suite"
echo "  binary: $MK"
"$MK" -V MAKE 2>/dev/null | sed 's/^/  mk reports: /' || true
echo

total=0
failed=0
faillist=""

# test_*.sh files in lexicographic order.
for f in "$TESTDIR"/test_*.sh; do
	[ -f "$f" ] || continue
	total=$((total + 1))
	name=$(basename "$f")
	if sh "$f"; then
		printf 'FILE %-28s PASS\n' "$name"
	else
		printf 'FILE %-28s FAIL\n' "$name"
		failed=$((failed + 1))
		faillist="$faillist $name"
	fi
done

echo
echo "------------------------------------------------------------"
if [ "$failed" -eq 0 ]; then
	echo "ALL $total FILES PASSED"
	exit 0
else
	echo "FAIL: $failed/$total files failed:${faillist}"
	exit 1
fi
