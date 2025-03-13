AC_PREREQ([2.52])
AC_INIT([mk], [0.0], [benni@stuerz.xyz])

# The configure test fails for Xenix, because their `grep` is slightly broken.
# This tells configure, to skip the `grep` tests.
AS_IF([test "x$(uname)" = 'xXENIX'], [
       ac_cv_path_GREP='/bin/grep'
])

AC_CONFIG_SRCDIR([mk.c])
AC_CONFIG_AUX_DIR([build-aux])
m4_include([m4/ax_compiler_vendor.m4])

# MK_CHECK_COMPILE_FLAG(FLAG)
AC_DEFUN([MK_CHECK_COMPILE_FLAG], [
	AS_VAR_PUSHDEF([CACHEVAR], [mk_cv_check_compile_flags_$1])
	AC_CACHE_CHECK([whether the C compiler accepts $1], CACHEVAR, [
		mk_check_save_flags="$CFLAGS"
		CFLAGS="$CFLAGS $1 -Werror"
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
			[AS_VAR_SET(CACHEVAR, [yes])],
			[AS_VAR_SET(CACHEVAR, [no])])
		CFLAGS="$mk_check_save_flags"
	])
	AS_IF([test x$CACHEVAR = xyes], [CFLAGS="$CFLAGS $1"], [:])
	AS_VAR_POPDEF([CACHEVAR])
])

# This is only available starting with autoconf-2.58
m4_define([AS_HELP_STRING],
[m4_pushdef([AS_Prefix], m4_default([$3], [                          ]))
m4_pushdef([AS_Prefix_Format],
	   [  %-]m4_eval(m4_len(AS_Prefix) - 3)[s ])
m4_text_wrap([$2], AS_Prefix, m4_format(AS_Prefix_Format, [$1]))
m4_popdef([AS_Prefix_Format])
m4_popdef([AS_Prefix])
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

AC_PROG_CC
AC_PROG_INSTALL

#AC_USE_SYSTEM_EXTENSIONS
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
	pcc)
		MK_CHECK_COMPILE_FLAG([-Wno-missing-prototypes])
		;;
	esac
], [
	MK_CHECK_COMPILE_FLAG([-w])
])

AC_FUNC_FORK
AC_CHECK_FUNCS([getcwd reallocarray fnmatch basename dirname strdup realpath strsep clock_gettime gettimeofday ftime fmemopen lstat memmove])
AC_CHECK_HEADERS([err.h fnmatch.h libgen.h limits.h sys/time.h sys/timeb.h sys/wait.h])
AC_CHECK_MEMBER([struct timespec.tv_nsec], [
	AC_DEFINE([HAVE_TIMESPEC], [1], [Define to 1 if you have the 'timespec' struct])
])
AC_CHECK_MEMBER([struct stat.st_mtim], [
	AC_DEFINE([HAVE_STAT_MTIM], [1], [Define to 1 if you have the 'st_mtim' field in 'struct stat'])
])
AC_CHECK_TYPE([ssize_t], [int])

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

AC_MSG_CHECKING([if $mk_shell supports -c cmd])
AS_IF(["$mk_shell" -c 'true' >/dev/null 2>&1], [mk_cv_shell_c=yes], [mk_cv_shell_c=no])
AC_MSG_RESULT([$mk_cv_shell_c])

AC_MSG_CHECKING([if $mk_shell supports -e])
"$mk_shell" -e -c 'true' >/dev/null 2>&1
mk_cv_shell_e1=$?
"$mk_shell" -e -c 'x() { return 5; }; x; true' >/dev/null 2>&1
mk_cv_shell_e2=$?
AS_IF([test "$mk_cv_shell_e1" = 0 && test "$mk_cv_shell_e2" = 5], [mk_cv_shell_e=yes], [mk_cv_shell_e=no])
AC_MSG_RESULT([$mk_cv_shell_e])

AS_IF([test "x$mk_cv_shell_c" != 'xyes'], [
	echo "$mk_shell is not a working shell" >&2
	exit 1
])

AS_IF([test "x$mk_cv_shell_e" != 'xyes'], [
	AC_DEFINE_UNQUOTED([HAVE_BROKEN_SHELL], [1], [Does your shell support '-e' properly?])
])

AC_DEFINE_UNQUOTED([SHELL], ["$mk_shell"], [Default value for SHELL variable])
AC_DEFINE_UNQUOTED([MAKEFILE], ["$mk_makefile"], [Default name of the makefile])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.mk])

AC_OUTPUT

sed	-e '/^-include config.mk/d'	\
	-e 's/^[a-zA-Z]* ?=.*$/# &/'	\
	< Mkfile > Makefile.new

cat config.mk Makefile.new > Makefile
