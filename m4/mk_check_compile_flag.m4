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
