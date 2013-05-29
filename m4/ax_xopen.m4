dnl Figure out what flags must be set for maximum _XOPEN_SOURCE support
dnl
dnl Jens Rehsack <sno@NetBSD.org>
dnl
dnl Covered:
dnl  XPG5 - X/Open CAE Specification, Issue 5 (XPG5/UNIX 98/SUSv2)
dnl  XPG6 - Open Group Technical Standard, Issue 6 (XPG6/UNIX 03/SUSv3)
dnl
dnl TODO: really check for _XOPEN_SOURCE (XPG3), XOPEN_SOURCE = 4 (XPG4)
dnl       and _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED = 1 (XPG4v2)

AC_DEFUN([AX_TRY_COMPILE_XOPEN_SOURCE],
[
  AC_COMPILE_IFELSE(
    [$1], [
      HAVE_XOPEN_SOURCE_LEVEL="$3"
      HAVE_XOPEN_SOURCE="$4"
      NEED_DEFINE_XOPEN_SOURCE=0
    ], [
      ax_safe_cppflags="${CPPFLAGS}"
      CPPFLAGS="${CPPFLAGS} $2"
      AC_COMPILE_IFELSE(
	[$1], [
	  HAVE_XOPEN_SOURCE_LEVEL="$3"
	  HAVE_XOPEN_SOURCE="$4"
	  NEED_DEFINE_XOPEN_SOURCE=1
	]
      )
      CPPFLAGS="${ax_safe_cppflags}"
    ]
  )
])

AC_DEFUN([AX_CHECK_XOPEN_SOURCE],
[
  dnl Checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])

  define(xpg7_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int
main ()
{
  int rc, i;
  struct dirent *d1 = NULL, *d2 = NULL;
  char *s1 = NULL, *s2 = NULL, *s3 = NULL;
  FILE *f = NULL;
  size_t len = 1024;
  ssize_t si;

  rc = alphasort( &d1, &d2 );
  rc = faccessat( 1, s1, R_OK, AT_EACCESS );
  rc = fexecve( 1, &s1, &s2 );
  f = fmemopen( s1, 1024, s2 );
  si = getdelim( &s1, &len, ' ', f );
  si = getline( &s1, &len, f );
  s3 = stpcpy( s1, s2 );
  s3 = stpncpy( s1, s2, len );
  len = strnlen( s1, 1024 );
  s2 = strndup( s1, 1024 );
  s1 = strsignal( SIGINT );

  return 0;
}
  ])])
  define(xpg6_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <unistd.h>

int
main ()
{
  int rc;
  char *s1 = NULL, *s2 = NULL;

  rc = setenv( s1, s2, 0 );
  rc = setegid( 0 );
  rc = seteuid( 0 );

  return 0;
}
  ])])
  define(xpg52_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <arpa/inet.h>

int
main ()
{
  int rc;
  char *s1 = NULL, *s2 = NULL;
  const char *addr;
  char addr_str[[64]];
  struct sockaddr_in sock_buf;

  rc = inet_pton( AF_INET, "127.0.0.1", &sock_buf );
  addr = inet_ntop( AF_INET, &sock_buf, addr_str, sizeof(addr_str) );

  return 0;
}
  ])])
  define(xpg5_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <unistd.h>
#include <utmpx.h>

int
main ()
{
  int rc;
  struct utmpx *utent;

  rc = ftruncate( 1, 1024 );
  rc = fsync( 1 );
  setutxent();
  utent = getutxent();
  endutxent();

  return 0;
}
  ])])
  define(xpg42_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <unistd.h>
#include <sys/resource.h>

int
main ()
{
  char buf[[1024]];
  size_t sz = sizeof(buf);
  int rc;
  struct rlimit rl;

  rc = gethostname( buf, sz );
  rc = getrlimit( RLIMIT_CORE, &rl );

  return 0;
}
  ])])
  define(xpg4_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <unistd.h>
#include <sys/wait.h>

int
main (int argc, char **argv)
{
  size_t len;
  char buf[[1024]];
  int ch, status = 0;
  pid_t child = 100;

  len = confstr( 1, buf, sizeof(buf ) );
  ch = getopt( argc, argv, "h" );
  rc = waitpid( child, &status, WNOHANG );

  return 0;
}
  ])])
  define(xpg3_testcode, [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <unistd.h>

int
main ()
{
  int rc;

  rc = nice( 5 );
  rc = chroot( "/tmp" );

  return 0;
}
  ])])

  AC_MSG_CHECKING([which _XOPEN_SOURCE macro must be defined])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg3_testcode], [-D_XOPEN_SOURCE], 300, [XPG3])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg4_testcode], [-D_XOPEN_SOURCE=4], 400, [XPG4])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg42_testcode], [-D_XOPEN_SOURE -D_XOPEN_SOURCE_EXTENDED], 420, [XPG4.2])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg5_testcode], [-D_XOPEN_SOURE=500], 500, [XPG5])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg52_testcode], [-D_XOPEN_SOURE=520], 520, [XPG5.2])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg6_testcode], [-D_XOPEN_SOURE=600], 600, [XPG6])
  AX_TRY_COMPILE_XOPEN_SOURCE([xpg7_testcode], [-D_XOPEN_SOURE=700], 700, [XPG7])
  AS_IF(
    [test "${NEED_DEFINE_XOPEN_SOURCE}" -eq "1"],
    [AS_IF(
      [test "X${HAVE_XOPEN_SOURCE}" = "XEXTENDED"],
      [
        dnl Need to define this manually for XPG42
        XOPEN_SOURCE_CPPFLAGS="-D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED"
      ],
      [
        dnl Need to enable this manually
        XOPEN_SOURCE_CPPFLAGS="-D_XOPEN_SOURCE=${HAVE_XOPEN_SOURCE}"
      ]
      AC_MSG_RESULT([$XOPEN_SOURCE_CPPFLAGS])
    )],
    [
      AC_MSG_RESULT([none ($HAVE_XOPEN_SOURCE)])
    ]
  )
  AC_DEFINE_UNQUOTED([HAVE_XOPEN_SOURCE], $HAVE_XOPEN_SOURCE_LEVEL, [_XOPEN_SOURCE $HAVE_XOPEN_SOURCE])
])
