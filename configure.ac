# TODO: --with-shell=sh, --disable-warnings
AC_INIT([mk], [0.0], [benni@stuerz.xyz])

AC_DEFUN([MK_CHECK_COMPILE_FLAG], [
	AX_CHECK_COMPILE_FLAG([$1], [CFLAGS="${CFLAGS} $1"])
])

AC_CONFIG_SRCDIR([mk.c])
AC_CONFIG_AUX_DIR([build-aux])

AC_PROG_CC

MK_CHECK_COMPILE_FLAG([-Wall])
MK_CHECK_COMPILE_FLAG([-Wextra])
MK_CHECK_COMPILE_FLAG([-Wno-deprecated-non-prototype])
MK_CHECK_COMPILE_FLAG([-Wno-implicit-int])
MK_CHECK_COMPILE_FLAG([-Wno-return-type])

AC_FUNC_FORK
AC_CHECK_FUNCS([reallocarray fnmatch basename dirname])
AC_CHECK_HEADERS([err.h fnmatch.h libgen.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.mk])

CPPFLAGS="${CPPFLAGS} -D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700"

AC_OUTPUT
