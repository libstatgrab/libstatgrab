dnl Find a printf/scanf format pattern for given type
dnl
dnl Motivation: get rid of error messages of wrong types when printing
dnl             process id's and similar
dnl
dnl Jens Rehsack <sno@NetBSD.org>

AC_DEFUN([AX_CHECK_TYPE_SIGN_CHECK], [AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$2])],
[  /* signed:	-1 / 2 -> 0
   *		0 % 4 -> 0
   *		0 - 1 -> -1
   * unsigned:	0xff / 2 -> 0x7f
   *		0x7f % 4 -> 3
   *		3 - 1 -> 2
   */
  int fmtchk[[((($1)-1)/2)%4-1]];
  unsigned fcs = sizeof(fmtchk);
  printf("%u\n", fcs); /* avoid -Wunused ... */
  ])
])

AC_DEFUN([AX_CHECK_TYPE_SIZE_CMP], [AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$3])],
  [int sizechk[[sizeof($1) == sizeof($2)]];
  unsigned scs = sizeof(sizechk);
  printf("%u\n", scs); /* avoid -Wunused ... */
  ])
])

AC_DEFUN([AX_CHECK_TYPE_FMT_CHECK], [AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$3])],
       [$1 test$1; sscanf("1234567890", "$2", &test$1); printf("$2\n", test$1);])
])

AC_DEFUN([AX_CHECK_TYPE_FMT], [
  define([Name],[translit([$1],[ ], [_])])
  define([NAME],[translit([$1], [ abcdefghijklmnopqrstuvwxyz], [_ABCDEFGHIJKLMNOPQRSTUVWXYZ])])
dnl AC_REQUIRE([AC_TYPE_][]NAME)dnl
  AC_CACHE_CHECK([for format string for $1], [ax_cv_type_fmt_]Name, [
    AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIGN_CHECK([$1], [$2])], [
      AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned int], [$2])], [
        AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%u], [$2])], [ax_cv_type_fmt_[]Name[]="%u"])], [
	AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%lu], [$2])], [ax_cv_type_fmt_[]Name[]="%lu"])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned long long], [$2])], [
	    AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%llu], [$2])], [ax_cv_type_fmt_[]Name[]="%llu"])])
	  ]
	)]
      )
    ], [
      AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [int], [$2])], [
        AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%d], [$2])], [ax_cv_type_fmt_[]Name[]="%d"])], [
	AC_LINK_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%ld], [$2])], [ax_cv_type_fmt_[]Name[]="%ld"])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [long long], [$2])], [
	    AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%lld], [$2])], [ax_cv_type_fmt_[]Name[]="%lld"])])
	  ]
	)]
      )
    ])
  ])

  AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [AC_MSG_WARN([Unable to find format string for $1])],
  [AC_DEFINE_UNQUOTED([FMT_[]NAME], ["$ax_cv_type_fmt_[]Name"], [Define FMT string for $1])])
])

AC_DEFUN([AX_CHECK_TYPES_FMT], [m4_foreach_w([AX_Type], [$1], [
AX_CHECK_TYPE_FMT(AX_Type, [$2])])])
