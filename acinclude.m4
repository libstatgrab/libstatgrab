dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/mp_with_curses.html
dnl (Hacked by tdb for libstatgrab)
dnl $Id$
AC_DEFINE([HAVE_CURSES_H], [], [Define to 1 if you have the <curses.h> header file.])
AC_DEFINE([HAVE_NCURSES_H], [], [Define to 1 if you have the <ncurses.h> header file.])
AC_DEFUN([MP_WITH_CURSES],
  [AC_ARG_WITH(ncurses, [  --with-ncurses          Force the use of ncurses over curses],,)
   mp_save_LIBS="$LIBS"
   mp_save_CPPFLAGS="$CPPFLAGS"
   LIBS="$LIBS $SAIDARLIBS"
   CPPFLAGS="$CPPFLAGS $SAIDARCPPFLAGS"
   CURSES_LIB=""
   if test "$with_ncurses" != yes
   then
     AC_CACHE_CHECK([for working curses], mp_cv_curses,
       [LIBS="$LIBS -lcurses"
        AC_TRY_LINK(
          [#include <curses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_curses=yes, mp_cv_curses=no)])
     if test "$mp_cv_curses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H)
       CURSES_LIB="-lcurses"
     fi
   fi
   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working ncurses], mp_cv_ncurses,
       [LIBS="$mp_save_LIBS -lncurses"
        AC_TRY_LINK(
          [#include <ncurses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_ncurses=yes, mp_cv_ncurses=no)])
     if test "$mp_cv_ncurses" = yes
     then
       AC_DEFINE(HAVE_NCURSES_H)
       CURSES_LIB="-lncurses"
     else
       AC_MSG_WARN([Unable to find curses or ncurses; disabling saidar])
       AM_CONDITIONAL(SAIDAR, false)
     fi
   fi
   SAIDARLIBS="$SAIDARLIBS $CURSES_LIB"
   LIBS="$mp_save_LIBS"
   CPPFLAGS="$mp_save_CPPFLAGS"
])dnl
