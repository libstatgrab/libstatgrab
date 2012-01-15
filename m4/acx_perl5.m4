dnl Find a suitable perl5 interpreter and perl modules
dnl
dnl Motivation: mixing perl tests into autoconf based distribution
dnl
dnl Jens Rehsack <sno@NetBSD.org>

AC_DEFUN([_ACX_BUILD_MODULE_TEST_CODE],
[define(
  [ModuleName],m4_ifval(regexp([$1], [^\([^ 	]+\)[ 	]+\(.*\)$], [\1]),regexp([$1], [^\([^ 	]+\)[ 	]+\(.*\)$], [\1]),$1))define(
  [ModuleVersion],regexp([$1], [^\([^ 	]+\)[ 	]+\(.*\)$], [\2]))m4_map([m4_echo], [
	[-e 'use ModuleName;],
	[m4_ifval(ModuleVersion, [ ModuleName->VERSION() > ModuleVersion or die "ModuleName ModuleVersion required--this is only " . ModuleName->VERSION();])],
	[' ]])])

AC_DEFUN([_ACX_PROG_PERL_ARGS], [
	AC_ARG_WITH([perl5],
[  --with-perl5[[=/path/to/perl5]]
  --without-perl5],
	[$as_test_x "$withval" && acx_perl5_prog="$withval"])
])

dnl ACX_PROG_PERL5([INTERPRETER-LIST], [INCLUDE-PATH], [MINIMUM-VERSION],
dnl                [MODULE-LIST], [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Declares configure options
dnl   --with-perl5[=/path/to/perl5]
dnl   --without-perl5
dnl If neither --with-perl5 nor --without-perl5 is specified for configure
dnl   run, the path is searched for binaries named 'perl5' or 'perl'

AC_DEFUN([ACX_PROG_PERL5],
[
	AC_REQUIRE([AC_PROG_GREP])
	AC_REQUIRE([AC_PROG_SED])
	AC_REQUIRE([_ACX_PROG_PERL_ARGS])

	AC_CACHE_CHECK([for applicable perl5 interpreter], [acx_cv_perl5_prog], [
		define([PERL5_INTERPRETER_LIST], m4_ifnblank([$1], $1, [perl5 perl]))
		define([PERL5_INCLUDES], m4_ifnblank([$2], m4_combine([ ], [-I], [], m4_do([m4_echo], m4_split([$2])))))
		define([PERL5_VERSION_CHECK], m4_ifnblank([$3], [-e 'use $3;']))
		define([PERL5_MODULES_CHECK], [m4_ifnblank([$4], m4_map_args_sep([_ACX_BUILD_MODULE_TEST_CODE(], [)], [], $4))])
		define([PERL5_CMDLINE], m4_normalize( m4_join([ ], PERL5_INCLUDES, PERL5_VERSION_CHECK, PERL5_MODULES_CHECK, [-V])))

		AC_PATH_PROGS_FEATURE_CHECK([acx_perl5_prog], PERL5_INTERPRETER_LIST, [
			output=`$ac_path_acx_perl5_prog PERL5_CMDLINE 2>/dev/null`
			AS_IF(	[test $? -eq 0], [
				acx_perl5_prog=m4_ifnblank( [PERL5_INCLUDES], "$ac_path_acx_perl5_prog PERL5_INCLUDES", "$ac_path_acx_perl5_prog" )
				ac_cv_path_acx_perl5_prog="$acx_perl5_prog"
				ac_path_acx_perl5_prog_found=:
				])
		], [acx_perl5_prog="none"])
		acx_cv_perl5_prog="$acx_perl5_prog"

		undefine([PERL5_INTERPRETER_LIST])
		undefine([PERL5_INCLUDES])
		undefine([PERL5_VERSION_CHECK])
		undefine([PERL5_MODULES_CHECK])
		undefine([PERL5_CMDLINE])
	])
	PERL5="$acx_cv_perl5_prog"
	AC_SUBST(PERL5)
	m4_ifnblank([$5], [AS_IF([test "x$PERL5" != "xnone"], $5, $6)])
])
