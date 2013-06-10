dnl define some modre 
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

AU_ALIAS([AC_TYPE_GID_T], [AC_TYPE_UID_T])
