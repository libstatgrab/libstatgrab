dnl make requirement AC_TYPE_TIME_T happy
AN_IDENTIFIER([time_t], [AC_TYPE_TIME_T])
AC_DEFUN([AC_TYPE_TIME_T], [AC_CHECK_TYPE([time_t], [long int], [], [], [#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif])])

dnl make requirement AC_TYPE_GID_T happy
AU_ALIAS([AC_TYPE_GID_T], [AC_TYPE_UID_T])

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
  [int sizechk[[(sizeof($1) == sizeof($2)) * 2 - 1]];
  unsigned scs = sizeof(sizechk);
  printf("%u\n", scs); /* avoid -Wunused ... */
  ])
])

AC_DEFUN([AX_CHECK_TYPE_FMT_CHECK], [AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$3])],
  [$1 test$1; sscanf("1234567890", "$2", &test$1); printf("$2\n", test$1);])
])

AC_DEFUN([AX_CHECK_TYPE_FMT], [
  define([Name],[translit([$1], [ ], [_])])
  define([NAME],[translit([$1], [ abcdefghijklmnopqrstuvwxyz], [_ABCDEFGHIJKLMNOPQRSTUVWXYZ])])
  AC_REQUIRE([AC_TYPE_][]NAME)dnl
  AC_CACHE_CHECK([for format string for $1], [ax_cv_type_fmt_]Name, [
    ax_save_[]_AC_LANG_ABBREV[]_werror_flag="$ac_[]_AC_LANG_ABBREV[]_werror_flag"
    AC_LANG_WERROR()
    AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIGN_CHECK([$1], [$2])], [
      AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned int], [$2])], [
        AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%u], [$2])], [ax_cv_type_fmt_[]Name[]="%u"])])
      AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [
	AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%lu], [$2])], [ax_cv_type_fmt_[]Name[]="%lu"])])])
      AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [
	AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [unsigned long long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%llu], [$2])], [ax_cv_type_fmt_[]Name[]="%llu"])])])
    ], [
      AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [int], [$2])], [
        AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%d], [$2])], [ax_cv_type_fmt_[]Name[]="%d"])])
      AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [
	AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%ld], [$2])], [ax_cv_type_fmt_[]Name[]="%ld"])])])
      AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [
	AC_COMPILE_IFELSE([AX_CHECK_TYPE_SIZE_CMP([$1], [long long], [$2])], [
	  AC_COMPILE_IFELSE([AX_CHECK_TYPE_FMT_CHECK([$1], [%lld], [$2])], [ax_cv_type_fmt_[]Name[]="%lld"])])])
    ])
    ac_[]_AC_LANG_ABBREV[]_werror_flag="$ax_save_[]_AC_LANG_ABBREV[]_werror_flag"
  ])

  AS_IF([test ! "$ax_cv_type_fmt_[]Name"], [AC_MSG_WARN([Unable to find format string for $1])],
  [AC_DEFINE_UNQUOTED([FMT_[]NAME], ["$ax_cv_type_fmt_[]Name"], [Define FMT string for $1])])
])

AC_DEFUN([AX_CHECK_TYPES_FMT], [m4_foreach_w([AX_Type], [$1], [
AX_CHECK_TYPE_FMT(AX_Type, [$2])])])

AC_DEFUN([AX_CHECK_ALIGNOF], [
  define([Name],[translit([$1], [ ], [_])])
  define([NAME],[translit([$1], [ abcdefghijklmnopqrstuvwxyz], [_ABCDEFGHIJKLMNOPQRSTUVWXYZ])])
  AC_REQUIRE([AC_TYPE_UNSIGNED_LONG_LONG_INT])
  AC_CACHE_CHECK([whether $1 needs alignment helper], [ax_cv_alignment_helper_]Name, [
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([AC_INCLUDES_DEFAULT([$2])
#if defined(__ICC) && defined(offsetof)
# undef offsetof
#endif

#ifndef offsetof
# define offsetof(type,memb) ((unsigned)(((char *)(&((type*)0)->memb)) - ((char *)0)))
#endif

typedef struct { $1 a;
#ifdef HAVE_UNSIGNED_LONG_LONG_INT
unsigned long long b;
#else
size_t b;
#endif
} ax_type_alignof_;],
    [int __ax_alignof[[(offsetof(ax_type_alignof_, b) == (unsigned long)((($1 *)0)+1)) * 2 - 1]];
     unsigned acs = sizeof(__ax_alignof);
     printf("%u\n", acs); /* avoid -Wunused ... */]
    )],
    [ax_cv_alignment_helper_[]Name[]="no"], [ax_cv_alignment_helper_[]Name[]="yes"])
  ])

  AS_IF([test "$ax_cv_alignment_helper_[]Name" = "no"],
    [AC_DEFINE([NAME[]_ALIGN_OK], [1], [Defined with a true value when $1 is well aligned])])
])
