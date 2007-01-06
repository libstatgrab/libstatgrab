dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/mp_with_curses.html
dnl (Hacked by tdb for libstatgrab)
dnl $Id$
AC_DEFINE([HAVE_CURSES_H], [], [Define to 1 if you have the <curses.h> header file.])
AC_DEFINE([HAVE_NCURSES_H], [], [Define to 1 if you have the <ncurses.h> header file.])
AC_DEFINE([CURSES_HEADER_FILE], [], [Set to the location of the curses header file.])
AC_DEFUN([MP_WITH_CURSES],
  [AC_ARG_WITH(ncurses, [  --without-ncurses       Do not use ncurses],,)

   mp_save_LIBS="$LIBS"
   mp_save_CPPFLAGS="$CPPFLAGS"
   CURSES_LIB=""

   if test "$with_ncurses" != no
   then
     AC_CACHE_CHECK([for working ncurses - ncurses.h], mp_cv_ncurses1,
       [LIBS="$mp_save_LIBS $SAIDARLIBS -lncurses"
        CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
        AC_TRY_LINK(
          [#include <ncurses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_ncurses1=yes, mp_cv_ncurses1=no)])
     if test "$mp_cv_ncurses1" = yes
     then
       AC_DEFINE([HAVE_NCURSES_H])
       AC_DEFINE([CURSES_HEADER_FILE], [<ncurses.h>])
       CURSES_LIB="-lncurses"
     else
       AC_CACHE_CHECK([for working ncurses - ncurses/ncurses.h], mp_cv_ncurses2,
         [LIBS="$mp_save_LIBS $SAIDARLIBS -lncurses"
          CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
          AC_TRY_LINK(
            [#include <ncurses/ncurses.h>],
            [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
            mp_cv_ncurses2=yes, mp_cv_ncurses2=no)])
       if test "$mp_cv_ncurses2" = yes
       then
         AC_DEFINE([HAVE_NCURSES_H])
         AC_DEFINE([CURSES_HEADER_FILE], [<ncurses/ncurses.h>])
         CURSES_LIB="-lncurses"
       fi
     fi
   fi

   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working curses], mp_cv_curses,
       [LIBS="$mp_save_LIBS $SAIDARLIBS -lcurses"
        CPPFLAGS="$mp_save_CPPFLAGS $SAIDARCPPFLAGS"
        AC_TRY_LINK(
          [#include <curses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          mp_cv_curses=yes, mp_cv_curses=no)])
     if test "$mp_cv_curses" = yes
     then
       AC_DEFINE([HAVE_CURSES_H])
       AC_DEFINE([CURSES_HEADER_FILE], [<curses.h>])
       CURSES_LIB="-lcurses"
     fi
   fi

   if test ! "$CURSES_LIB"
   then
     AC_MSG_WARN([Unable to find curses or ncurses; disabling saidar])
     AM_CONDITIONAL(SAIDAR, false)
   fi

   SAIDARLIBS="$SAIDARLIBS $CURSES_LIB"
   LIBS="$mp_save_LIBS"
   CPPFLAGS="$mp_save_CPPFLAGS"
])dnl
