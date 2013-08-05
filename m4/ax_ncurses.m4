dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/mp_with_curses.html
dnl (Hacked by tdb for libstatgrab)
dnl $Id$
AC_DEFUN([MP_WITH_CURSES], [
  AC_ARG_WITH(ncurses, [  --without-ncurses       Do not use ncurses],,)

  mp_save_LIBS="$LIBS"
  mp_save_CPPFLAGS="$CPPFLAGS"
  CURSES_LIB=""
  define([testcode], [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); addchnstr(&a, b); move(b,c);])

  AS_IF([test "x$with_ncurses" != "xno"], [
    AC_CACHE_CHECK([for working ncurses], mp_cv_ncurses, [
      LIBS="$mp_save_LIBS $SAIDARLIBS -lncurses"
      CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <ncurses.h>], [testcode])], [
        mp_cv_ncurses="ncurses.h"
        CURSES_LIB="-lncurses"
      ], [
        LIBS="$mp_save_LIBS $SAIDARLIBS -lncurses"
        CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
        AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <ncurses/ncurses.h>], [testcode])], [
          mp_cv_ncurses="ncurses/ncurses.h"
          CURSES_LIB="-lncurses"
        ], [
          LIBS="$mp_save_LIBS $SAIDARLIBS -lcurses"
          CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
          AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <curses.h>], [testcode])], [
            mp_cv_ncurses="curses.h"
            CURSES_LIB="-lcurses"
          ], [mp_cv_ncurses=no])
        ])
      ])
    ])
  ])

  AS_IF([test ! "$CURSES_LIB"], [
    AC_MSG_WARN([Unable to find curses or ncurses; disabling saidar])
    AM_CONDITIONAL(SAIDAR, false)
  ], [test "$mp_cv_ncurses" = "ncurses.h"], [
    AC_DEFINE([HAVE_NCURSES_H], 1, [Define to 1 if you have the <ncurses.h> header file.])
  ], [test "$mp_cv_ncurses" = "ncurses/ncurses.h"], [
    AC_DEFINE([HAVE_NCURSES_NCURSES_H], 1, [Define to 1 if you have the <ncurses/ncurses.h> header file.])
  ], [test "$mp_cv_ncurses" = "curses.h"], [
    AC_DEFINE([HAVE_CURSES_H], 1, [Define to 1 if you have the <curses.h> header file.])
  ])

  SAIDARLIBS="$SAIDARLIBS $CURSES_LIB"
  LIBS="$mp_save_LIBS"
  CPPFLAGS="$mp_save_CPPFLAGS"
])dnl
