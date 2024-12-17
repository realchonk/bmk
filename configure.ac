AC_INIT([mk], [0.0], [benni@stuerz.xyz])

AC_CONFIG_SRCDIR([mk.c])
AC_CONFIG_AUX_DIR([build-aux])

AC_PROG_CC

AC_FUNC_FORK
AC_CHECK_FUNCS([reallocarray fnmatch basename dirname])
AC_CHECK_HEADERS([err.h fnmatch.h libgen.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.mk])

AC_OUTPUT
