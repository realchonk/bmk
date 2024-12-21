# TODO: --with-shell=sh, --disable-warnings
AC_INIT([mk], [0.0], [benni@stuerz.xyz])

AC_DEFUN([MK_CHECK_COMPILE_FLAG], [
	AX_CHECK_COMPILE_FLAG([$1], [CFLAGS="${CFLAGS} $1"])
])

AC_ARG_ENABLE([warnings], AS_HELP_STRING([--disable-warnings], [disable compiler warnings]))
AC_ARG_WITH([shell],
	AS_HELP_STRING([--with-shell=SHELL], [default value for the SHELL variable (default: sh)]),
	[mk_shell=$with_shell],
	[mk_shell="sh"])
AC_ARG_WITH([makefile],
	AS_HELP_STRING([--with-makefile=FILE], [default name of the makefile (default: Mkfile)]),
	[mk_makefile=$with_makefile],
	[mk_makefile="Mkfile"])


AC_CONFIG_SRCDIR([mk.c])
AC_CONFIG_AUX_DIR([build-aux])

AC_PROG_CC

AC_USE_SYSTEM_EXTENSIONS
MK_CHECK_COMPILE_FLAG([-ansi])

AS_IF([test "x$enable_warnings" != "xno"], [
	MK_CHECK_COMPILE_FLAG([-Wall])
	MK_CHECK_COMPILE_FLAG([-Wextra])
	MK_CHECK_COMPILE_FLAG([-Wno-implicit-int])
	
	AX_COMPILER_VENDOR
	case "${ax_cv_c_compiler_vendor}" in
	gnu)
		MK_CHECK_COMPILE_FLAG([-Wno-return-type])
		MK_CHECK_COMPILE_FLAG([-Wno-missing-parameter-type])
		;;
	clang)
		MK_CHECK_COMPILE_FLAG([-Wno-deprecated-non-prototype])
		;;
	esac
], [
	MK_CHECK_COMPILE_FLAG([-w])
])

AC_FUNC_FORK
AC_CHECK_FUNCS([reallocarray fnmatch basename dirname strdup realpath strsep clock_gettime])
AC_CHECK_HEADERS([err.h fnmatch.h libgen.h sys/time.h])
AC_CHECK_MEMBER([struct timespec.tv_nsec], [
	AC_DEFINE([HAVE_TIMESPEC], [1], [Define to 1 if you have the 'timespec' struct])
])
AC_CHECK_MEMBER([struct stat.st_mtim], [
	AC_DEFINE([HAVE_STAT_MTIM], [1], [Define to 1 if you have the 'st_mtim' field in 'struct stat'])
])

AC_MSG_CHECKING([if the C compiler supports C99's designated initializers])
mk_have_designated_initializers=no
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([[
		struct test {
			char *name;
			int value;
		};
		static struct test obj = { .name = "test", .value = 42 };
	]])
], [
	mk_have_designated_initializers=yes
	AC_DEFINE([HAVE_DESIGNATED_INITIALIZERS], [1],
		  [Define to 1 if your compiler supports C99s designated initializers])
])
AC_MSG_RESULT([$mk_have_designated_initializers])

AS_IF([test "x$ac_cv_func_clock_gettime" = "xno"], [
	AC_CHECK_FUNC([gettimeofday])
])

AC_DEFINE_UNQUOTED([SHELL], ["$mk_shell"], [Default value for SHELL variable])
AC_DEFINE_UNQUOTED([MAKEFILE], ["$mk_makefile"], [Default name of the makefile])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.mk])

AC_OUTPUT
