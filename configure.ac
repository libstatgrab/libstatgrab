#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# configure.in for libstatgrab
# $Id$
#

# Might work with older autoconfs... but tested on 2.57
AC_PREREQ([2.69])

# Change these to change the package name and version
AC_INIT([libstatgrab],[0.90-rc2],[https://github.com/i-scream/libstatgrab/issues],,[http://www.i-scream.org/libstatgrab/])
AC_CONFIG_SRCDIR([src/libstatgrab/cpu_stats.c])
AM_INIT_AUTOMAKE()
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
# The file we use to pass #define's to the source
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE([disable])

# Revision number (automatically updated)
AC_REVISION($Revision$)

dnl
dnl Get cannonical host - the only one of build/host/target we care about
dnl build is where we build, host the the machine we build for
dnl
AC_CANONICAL_HOST
dnl and store it for generic error message instead of lots of ifdef's
AC_DEFINE_UNQUOTED([OS_TYPE], "$host_os",
                   [Define to be the name of the operating system.])

dnl AM_CONFIG_HEADER([config.h])

dnl disable unnecessary compiler checks
AC_DEFUN([AC_PROG_F77], [:])
AC_DEFUN([AC_PROG_FC], [:])
AC_DEFUN([AC_PROG_OBJC], [:])
AC_DEFUN([AC_PROG_OBJCCPP], [:])
AC_DEFUN([AC_LIBTOOL_F77], [:])

dnl disable check for GNU compatible malloc - just a working malloc is enough
AC_DEFUN([AC_FUNC_MALLOC_IF], [:])
AC_DEFUN([AC_FUNC_MALLOC], [
	AC_CHECK_FUNCS([malloc realloc])
])

# Checks for programs
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_SED
AC_PROG_GREP
AM_PROG_AR

# define default language
AC_LANG([C])

# Checks for standard C header files
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADER_STDBOOL
AS_IF([test "x$ac_cv_header_stdc" != "xyes"], [
  AC_CHECK_HEADERS([assert.h ctype.h errno.h float.h limits.h locale.h math.h setjmp.h signal.h stdarg.h stddef.h stdlib.h string.h])
])
ac_includes_default="\
${ac_includes_default}
#ifndef __cplusplus
# ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
# endif
#endif"

# Checks for typedefs, structures, and compiler characteristics
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_UID_T
AC_TYPE_GID_T
AC_TYPE_TIME_T
AC_TYPE_LONG_LONG_INT
AC_TYPE_UNSIGNED_LONG_LONG_INT

dnl Check for extra flags to enable some kind of behavior
AC_CHECK_DECL([_AIX],[ac_cv_need_minus_d_linux_source_compat="yes"; ac_cv_need_minus_d_all_source="yes"])
AC_CHECK_DECL([__linux],[ac_cv_need_minus_d_gnu_source="yes"]) dnl probably required by kfreebsd, too?
AC_CHECK_DECL([__sun],[ac_cv_need_minus_d_extensions="yes"; ac_cv_need_minus_d_posix_pthread_semantics="yes"])
AC_CHECK_DECL([__hpux],[ac_cv_need_minus_d_hpux_source="yes"])

AS_IF([test "x$ac_cv_need_minus_d_linux_source_compat" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_LINUX_SOURCE_COMPAT"])

AS_IF([test "x$ac_cv_need_minus_d_gnu_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_all_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_extensions" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__"])

AS_IF([test "x$ac_cv_need_minus_d_posix_pthread_semantics" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_POSIX_PTHREAD_SEMANTICS"])

AS_IF([test "x$ac_cv_need_minus_d_hpux_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_HPUX_SOURCE"])

dnl check language / compatibility environment
AX_CHECK_XOPEN_SOURCE
AS_IF([test -n "$XOPEN_SOURCE_CPPFLAGS"], [CPPFLAGS="$CPPFLAGS $XOPEN_SOURCE_CPPFLAGS"])

dnl Checks for inet libraries:
AC_SEARCH_LIBS(gethostent, [nsl])
AC_SEARCH_LIBS(setsockopt, [socket net network])
dnl AC_SEARCH_LIBS(connect, [inet])

# Checks for library functions
AC_FUNC_MALLOC
AC_FUNC_GETMNTENT
AC_CHECK_FUNCS([atoll bzero fcntl flock getloadavg getmntent getmntent_r seteuid setegid setresuid setresgid statfs statvfs statvfs64 strerror_r strndup strnlen vsprintf vsnprintf])

# Check for some additional headers
AC_CHECK_HEADERS([fcntl.h inttypes.h libgen.h limits.h netdb.h regex.h sched.h signal.h stdint.h sys/ioctl.h sys/socket.h sys/statvfs.h sys/unistd.h])

AC_CHECK_DECL([F_SETLK], [AC_DEFINE([HAVE_DECL_F_SETLK], 1, [define when F_SETLK is declared])], ,[
AC_INCLUDES_DEFAULT
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
])

AX_WIN32(
	[
		AC_CHECK_HEADERS([io.h Iphlpapi.h lm.h pdh.h process.h psapi.h winsock2.h ws2tcpip.h wspiapi.h winsock.h])
		AC_CHECK_FUNCS([strerror_s])
	],
	[ dnl else
		AC_HEADER_DIRENT
		AC_CHECK_HEADERS([libdevinfo.h mach/machine.h mntent.h paths.h sys/fstyp.h sys/loadavg.h sys/mnttent.h sys/mnttab.h sys/un.h sys/utsname.h sys/vfs.h utmp.h utmpx.h])
		AC_CHECK_HEADERS([sys/param.h sys/mount.h sys/proc.h sys/swap.h sys/sysctl.h sys/systeminfo.h sys/user.h], , , [
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
		])
		AC_CHECK_HEADERS([arpa/inet.h netinet/in.h net/if.h netinet/if_ether.h], , , [
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
		])
		AC_CHECK_HEADERS([sys/sar.h sys/sysinfo.h], , , [
		AC_INCLUDES_DEFAULT
#if HAVE_SYS_SAR_H
# include <sys/sar.h>
#endif
		])
		AC_CHECK_FUNCS([strncpy])
	]
)

AX_CHECK_TYPES_FMT([pid_t uid_t gid_t time_t])

AX_CHECK_ALIGNOF([struct sg_vector], [AC_INCLUDES_DEFAULT
struct sg_vector_init_info {
        size_t item_size;
        void * init_fn;
        void * copy_fn;
        void * compute_diff_fn;
        void * compare_fn;
        void * destroy_fn;
};
#ifndef NDEBUG
struct sg_vector {
        size_t start_eye;
        size_t used_count;
        size_t alloc_count;
        size_t block_shift;
        struct sg_vector_init_info info;
        size_t final_eye;
};
#else
struct sg_vector {
        size_t used_count;
        size_t alloc_count;
        size_t block_shift;
        struct sg_vector_init_info info;
};
#endif
])

AC_CACHE_CHECK([for unspecified array size for last struct member], ac_cv_unspecified_size_last_struct_member, [
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT

struct foobar {
    char *nm;
    unsigned nums[[]];
};

int main(void) {
	struct foobar *fbs;
	return 0;
}])], [ac_cv_unspecified_size_last_struct_member="yes"],
      [ac_cv_unspecified_size_last_struct_member="no"])
])
AS_IF([test "x$ac_cv_unspecified_size_last_struct_member" == "xyes"],
      [AC_DEFINE([HAVE_OPEN_ARRAY_AT_STRUCT_END], 1, [define this when array at struct end might be open])])

AC_CACHE_CHECK([for array length of 0], ac_cv_zero_array_size, [
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT

struct foobar {
    char *nm;
    unsigned nums[[0]];
};

int main(void) {
	struct foobar *fbs;
	return 0;
}])], [ac_cv_zero_array_size="yes"],
      [ac_cv_zero_array_size="no"])
])
AS_IF([test "x$ac_cv_zero_array_size" == "xyes"],
      [AC_DEFINE([HAVE_ZERO_SIZED_ARRAY], 1, [define this when array can have zero length])])

AC_CHECK_TYPES([struct statvfs], , , [
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
	])
AS_IF([test "x$ac_cv_type_struct_statvfs" = "xyes"], [
	# NetBSD 2.0 provides extended statvfs
	AC_CHECK_MEMBER(struct statvfs.f_fstypename,
			[AC_DEFINE(HAVE_STATVFS_FSTYPENAME, , [NetBSD extended statvfs])],
			[],
			[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
			])
	# Solaris, HP-UX and AIX, too - but other extension
	AC_CHECK_MEMBER(struct statvfs.f_basetype,
			[AC_DEFINE(HAVE_STATVFS_FBASETYPE, , [Solaris etc. extended statvfs])],
			[],
			[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
			])
	AC_CHECK_MEMBER(struct statvfs.f_iosize,
			[AC_DEFINE(HAVE_STATVFS_FIOSIZE, , [another BSD extension])],
			[],
			[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
			])
	AC_CHECK_MEMBER(struct statvfs.f_frsize,
			[AC_DEFINE(HAVE_STATVFS_FFRSIZE, , [another Solaris extension])],
			[],
			[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
			])

	AC_CHECK_TYPES([struct statvfs64], , , [
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
		])
	AS_IF([test "x$ac_cv_type_struct_statvfs64" = "xyes"], [
		# NetBSD 2.0 provides extended statvfs - does anyone copy this to an statvfs64?
		AC_CHECK_MEMBER(struct statvfs64.f_fstypename,
				[AC_DEFINE(HAVE_STATVFS64_FSTYPENAME, , [NetBSD extended statvfs64])],
				[],
				[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
				])
		# Solaris, HP-UX and AIX, too - but other extension
		AC_CHECK_MEMBER(struct statvfs64.f_basetype,
				[AC_DEFINE(HAVE_STATVFS64_FBASETYPE, , [Solaris etc. extended statvfs64])],
				[],
				[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
				])
		AC_CHECK_MEMBER(struct statvfs64.f_iosize,
				[AC_DEFINE(HAVE_STATVFS64_FIOSIZE, , [another BSD extension])],
				[],
				[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
				])
		AC_CHECK_MEMBER(struct statvfs64.f_frsize,
				[AC_DEFINE(HAVE_STATVFS64_FFRSIZE, , [another Solaris extension])],
				[],
				[
AC_INCLUDES_DEFAULT
#include <sys/statvfs.h>
				])
	])
])

AC_CHECK_FUNCS([sysfs])

AS_IF([test "x$ac_cv_header_utmpx_h" = "xyes"], [
	AC_CHECK_FUNCS([getutxent setutxent endutxent])
	AC_CHECK_DECLS([getutxent, setutxent, endutxent], , , [AC_INCLUDES_DEFAULT()
#include <utmpx.h>
])

	# FreeBSD 9 (and others) uses utmpx rather than utmp
	AC_CHECK_TYPES([struct utmpx], , , [#include <utmpx.h>])
	AC_CHECK_MEMBER(struct utmpx.ut_type,
			[AC_DEFINE(HAVE_UTMPX, , [utmpx])],
			[],
			[#include <utmpx.h>])

	# Some OS support ut_host, too
	AC_CHECK_MEMBER(struct utmpx.ut_host,
			[AC_DEFINE(HAVE_UTMPX_HOST, , [utmpx.ut_host])],
			[],
			[#include <utmpx.h>])

	# ... or have a "significant length of ut_host"
	AC_CHECK_MEMBER(struct utmpx.ut_syslen,
			[AC_DEFINE(HAVE_UTMPX_SYSLEN, , [utmpx.ut_syslen])],
			[],
			[#include <utmpx.h>])

	AC_CHECK_SIZEOF([utmpx.ut_id], ,[#include <utmpx.h>])
])

AS_IF([test "x$ac_cv_header_utmp_h" = "xyes"], [
	AC_CHECK_FUNCS([setutent endutent getutent])
	AC_CHECK_DECLS([getutent, setutent, endutent], , , [AC_INCLUDES_DEFAULT()
#include <utmp.h>
])

	AC_CHECK_TYPES([struct utmp], , , [
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)
	AC_CHECK_MEMBER(struct utmp.ut_line,
			[AC_DEFINE(HAVE_UTMP, , [utmp])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_type,
			[AC_DEFINE(HAVE_UTMP_TYPE, , [utmp.ut_type])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_name,
			[AC_DEFINE(HAVE_UTMP_NAME, , [utmp.ut_name])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_user,
			[AC_DEFINE(HAVE_UTMP_USER, , [utmp.ut_user])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_host,
			[AC_DEFINE(HAVE_UTMP_HOST, , [utmp.ut_host])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_pid,
			[AC_DEFINE(HAVE_UTMP_PID, , [utmp.ut_pid])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_id,
			[AC_DEFINE(HAVE_UTMP_ID, , [utmp.ut_id])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_MEMBER(struct utmp.ut_time,
			[AC_DEFINE(HAVE_UTMP_TIME, , [utmp.ut_time])],
			[],
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)

	AC_CHECK_SIZEOF([utmp.ut_id], ,
			[
AC_INCLUDES_DEFAULT

#include <utmp.h>
			]
	)
])

ax_include_sockets="\
#if HAVE_WINSOCK2_H
# include <winsock2.h>
# if HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif
# if HAVE_WSPIAPI_H
#  include <wspiapi.h>
# endif
#elif HAVE_WINSOCK_H
  /* IIRC winsock.h must be included before windows.h */
# include <winsock.h>
#else
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif"

AC_CHECK_MEMBERS([struct sockaddr_storage.ss_family, struct sockaddr_storage.__ss_family], [], [], [
AC_INCLUDES_DEFAULT
$ax_include_sockets
])

# Default needing setgid/setuid to false
AM_CONDITIONAL(SETGIDBINS, false)
AM_CONDITIONAL(SETUIDBINS, false)

# Default MINGW setting
AM_CONDITIONAL(MINGW, false)

ENABLE_THREADS_DEFAULT="check"

# Set things up for different OS's
# We define the name of the OS so the code can act accordingly
# We also need to add the right LDFLAGS
AS_CASE([$host_os],
	[solaris*], [
		AC_CHECK_HEADERS([kstat.h libdevinfo.h procfs.h sys/sockio.h])
		AC_DEFINE(SOLARIS, 1, [Building on Solaris])
		AM_CONDITIONAL(SETUIDBINS, true)
		BIN_OWNER="root"
		BIN_PERM="4755"
		AC_CHECK_LIB(kstat, kstat_open, [LINKFLAGS="$LINKFLAGS -lkstat"],[AC_MSG_ERROR([need libkstat])])
		AC_CHECK_LIB(devinfo, di_init, [LINKFLAGS="$LINKFLAGS -ldevinfo"])
		AC_CHECK_LIB(m, fabs, [SAIDARLIBS="$SAIDARLIBS -lm"])
		dnl check whether procfs is really available
		AC_DEFINE(HAVE_PROCFS, 1, [Define to signal that procfs is available])
		AC_CHECK_TYPES([struct swapent, struct swaptable, struct anoninfo], , , [
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_SWAP_H
# include <sys/swap.h>
#endif
		])
		ac_cv_threadsafe_strerror="yes"
	],
	[hpux11.11*|hpux11.[[23]]*], [
		AC_DEFINE(HPUX, , [Building on HP-UX 11.11+])
		AC_DEFINE(HPUX11, , [Building on HP-UX 11.11+])
		AC_CHECK_HEADERS([sys/pstat.h],,[AC_MSG_ERROR(need sys/pstat.h)])
		AC_CHECK_HEADERS([sys/dk.h sys/dlpi.h sys/dlpi_ext.h sys/mib.h sys/stropts.h])
		CPPFLAGS="$CPPFLAGS -D_PSTAT64"
		ac_cv_threadsafe_strerror="yes"
		ac_cv_threadsafe_getmntent="yes"
		AC_CHECK_MEMBER(struct pst_diskinfo.psd_dkbytewrite,
				[AC_DEFINE(HAVE_PST_DISKINFO_PSD_DKBYTEWRITE, , [pst_diskinfo.psd_dkbytewrite])],
				[],
				[#include <sys/pstat.h>])
		AC_CHECK_LIB(m, fabs, [SAIDARLIBS="$SAIDARLIBS -lm"])
	],
	[aix*], [
		AC_DEFINE(AIX, , [Building on AIX 5.x - 9.x])
		AC_CHECK_LIB(perfstat, perfstat_cpu_total, [LINKFLAGS="$LINKFLAGS -lperfstat"], [need libperfstat])
		AC_CHECK_HEADERS([sys/dr.h sys/protosw.h libperfstat.h procinfo.h sys/mntctl.h sys/statfs.h sys/vmount.h], , ,[
		AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_DR_H
#include <sys/dr.h>
#endif
#ifdef HAVE_SYS_PROTOSW_H
#include <sys/protosw.h>
#endif
		])
		AC_CHECK_FUNCS([mntctl])
		AC_CHECK_DECL([mntctl], [AC_DEFINE([HAVE_DECL_MNTCTL], 1, [define when mntctl() is declared properly])], ,[
		AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_MNTCTL_H
# include <sys/mntctl.h>
#endif
#ifdef HAVE_SYS_VMOUNT_H
#include <sys/vmount.h>
#endif
		])
		AC_CHECK_DECL([getprocs64], [AC_DEFINE([HAVE_DECL_GETPROCS64], 1, [define when getprocs64() is declared properly])], ,[
		AC_INCLUDES_DEFAULT
#ifdef HAVE_PROCINFO_H
# include <procinfo.h>
#endif
		])
		AC_CHECK_DECL([getargs], [AC_DEFINE([HAVE_DECL_GETARGS], 1, [define when getargs() is declared properly])], ,[
		AC_INCLUDES_DEFAULT
#ifdef HAVE_PROCINFO_H
# include <procinfo.h>
#endif
		])
	],
	[freebsd4.*], [
		AC_DEFINE(FREEBSD, , [Building on FreeBSD])
	],
	[freebsd[[5-9]].*|freebsd10.*], [
		AC_DEFINE(FREEBSD, , [Building on FreeBSD])
		AC_DEFINE(FREEBSD5, , [Building on FreeBSD 5.x - 10.x])
	],
	[dragonfly*], [
		AC_DEFINE(DFBSD, , [Building on DragonFlyBSD])
		ENABLE_THREADS_DEFAULT="no"
	],
	[kfreebsd*-gnu*], [
		AC_DEFINE(FREEBSD, , [Building on FreeBSD])
		AC_DEFINE(FREEBSD5, , [Building on FreeBSD 5.x - 10.x])
	],
	[darwin*], [
		AC_DEFINE(DARWIN, , [Building on Darwin])
		dnl following 2 are assumed because Darwin was derived from FreeBSD5 - prove it
		AC_DEFINE(FREEBSD, , [Building on FreeBSD])
		AC_DEFINE(FREEBSD5, , [Building on FreeBSD 5.x - 10.x])
		AX_APPEND_TO_VAR([LINKFLAGS], ["-framework IOKit" "-framework CoreFoundation"])
		#LINKFLAGS="${LINKFLAGS} -framework IOKit"
	],
	[netbsd*], [
		AC_DEFINE(NETBSD, , [Building on NetBSD])
	],
	[openbsd*], [
		AC_DEFINE(OPENBSD, , [Building on OpenBSD])
	],
	[linux-gnu*], [
		AC_DEFINE(LINUX, , [Building on GNU/Linux])
		AM_CONDITIONAL(SETUIDBINS, true)
		BIN_OWNER="root"
		BIN_PERM="4755"
		AC_DEFINE(HAVE_PROCFS, 1, [Define to signal that procfs is available])
		AC_CHECK_HEADERS([linux/ethtool.h linux/sockios.h])
		AS_IF([test "x$ac_cv_header_linux_ethtool_h" != "xyes"], [
		    unset ac_cv_header_linux_ethtool_h
		    ac_safe_cppflags="$CPPFLAGS"
		    CPPFLAGS="-D__u8=uint8_t -D__u16=uint16_t -D__u32=uint32_t -D__u64=uint64_t $CPPFLAGS"
		    AC_CHECK_HEADER([linux/ethtool.h])
		    CPPFLAGS="$ac_safe_cppflags"
		    AS_IF([test "x$ac_cv_header_linux_ethtool_h" = "xyes"],
		          [AC_DEFINE(LINUX_BROKEN_ETHTOOL_TYPES, 1, [define when linux/ethtool.h has broken types])])
		])
		AS_IF([test "x$ac_cv_header_linux_ethtool_h" = "xyes"], [
		    AC_CHECK_MEMBER(struct ethtool_cmd.speed_hi,
				    [AC_DEFINE(HAVE_ETHTOOL_CMD_SPEED_HI, 1, [struct ethtool_cmd.speed_hi available])],
				    [],
				    [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef LINUX_BROKEN_ETHTOOL_TYPES
# define __u8 uint8_t
# define __u16 uint16_t
# define __u32 uint32_t
# define __u64 uint64_t
# include <linux/ethtool.h>
# undef __u8
# undef __u16
# undef __u32
# undef __u64
#endif
#ifdef HAVE_LINUX_ETHTOOL_H
# include <linux/ethtool.h>
#endif
				    ]
		    )])
		])
	],
	[cygwin], [
		AC_DEFINE(CYGWIN, , [Building on Cygwin])
	],
	[mingw32], [
		AC_DEFINE(MINGW, , [Building on MinGW])
		AC_DEFINE(WINVER, 0x0500, [Building for Windows 2000])
		AM_CONDITIONAL(MINGW, true)
		LINKFLAGS="-lpdh -lIphlpapi -lpsapi -lnetapi32"
	],
	[
		AC_MSG_WARN([Build on unknown OS: $host_os - many functions might be not functional])
	]
)

dnl check Mach OS's
AS_CASE([$host_os],
	dnl Darwin, probably OSF/1, Plan9?, ...
	[darwin*], [
		AC_CHECK_HEADERS([mach/mach.h])
		
		AS_IF([test "x$ac_cv_header_mach_mach_h" != "xyes"], [AC_CHECK_HEADERS([mach/mach_host.h mach/vm_statistics.h mach/kern_return.h mach/host_info.h])])

		dnl Part of Mach Kernel Interface
		AC_CHECK_FUNCS([host_statistics host_statistics64])
	]
)

dnl check BSD* OS's
AS_CASE([$host_os],
	[*bsd*|darwin*|dragonfly*], [
		AC_DEFINE(ALLBSD, , [Building on some kind of BSD])
		AC_MSG_CHECKING([for BSD specific features])
		AC_MSG_RESULT([])

		AC_CHECK_HEADERS([ifaddrs.h kinfo.h net/if_media.h sys/dkstat.h sys/disk.h sys/iostat.h sys/resource.h sys/sched.h sys/ucred.h sys/user.h uvm/uvm.h sys/vmmeter.h vm/vm_param.h], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
		])])
		dnl FBSD 5+
		AC_CHECK_LIB(devstat, devstat_getnumdevs, [LINKFLAGS="$LINKFLAGS -ldevstat"])
		dnl FBSD < 5, DragonFlyBSD?, ...?
		AC_CHECK_LIB(devstat, getnumdevs, [LINKFLAGS="$LINKFLAGS -ldevstat"])
		AC_CHECK_HEADERS([devstat.h], , , [AC_LANG_SOURCE([
		AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
		])])

		AC_CHECK_LIB(kinfo, kinfo_get_sched_cputime, [LINKFLAGS="$LINKFLAGS -lkinfo"])

		ax_save_libs="$LIBS"
		LIBS="$LIBS $LINKFLAGS"
		AC_CHECK_FUNCS([getfsstat devstat_getdevs devstat_selectdevs getdevs kinfo_get_sched_cputime selectdevs sysctlbyname])
		LIBS="$ax_save_libs"

		AC_CACHE_CHECK([for properly declared and callable getvfsstat], [ac_cv_func_getvfsstat],
			ac_cv_func_getvfsstat=no
			AC_LINK_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#endif

int
main()
{
	struct statvfs buf;
	getvfsstat(&buf, sizeof(buf), 0);
}
				])],
				[
					AC_DEFINE([HAVE_GETVFSSTAT], 1, [define when getvfsstat() is declared properly])
					ac_cv_func_getvfsstat=yes
				]
			)
		)

		AC_CHECK_TYPES([struct kinfo_proc, struct kinfo_proc2, struct io_sysctl, struct diskstats, struct disk_sysctl, struct devstat, struct statinfo], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_DKSTAT_H
# include <sys/dkstat.h>
#endif
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_DEVSTAT_H
# include <devstat.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
		])])

		AC_CHECK_TYPES([struct kinfo_cputime], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
		])])

		AS_IF(	[test "x$ac_cv_type_struct_io_sysctl" = "xyes"], [
			AC_CHECK_MEMBER(struct io_sysctl.name,
					[AC_DEFINE(HAVE_IO_SYSCTL_NAME, 1, [struct io_sysctl.name available])],
					[AC_MSG_WARN([Probably better stop building libstatgrab when playing with statistic API])],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <sys/iostat.h>
					]
			)])
		])
		AS_IF(	[test "x$ac_cv_type_struct_disk_sysctl" = "xyes"], [
			# Disk IO read and write statistics are only present on recent NetBSD
			# read: everyting beyond NetBSD 1.6
			AC_CHECK_MEMBER(struct disk_sysctl.dk_rbytes,
					[AC_DEFINE(HAVE_DISK_SYSCTL_DK_RBYTES, , [New-style NetBSD disk stats])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <sys/disk.h>
					]
			)])
			AC_CHECK_MEMBER(struct disk_sysctl.dk_name,
					[AC_DEFINE(HAVE_DISK_SYSCTL_DK_RBYTES, , [New-style NetBSD disk stats])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <sys/disk.h>
					]
			)])
		])

		AS_IF(	[test "x$ac_cv_type_struct_devstat" = "xyes"], [
			AC_CHECK_MEMBER(struct devstat.bytes,
					[AC_DEFINE(HAVE_DEVSTAT_BYTES, 1, [struct devstat.bytes array available])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_DEVSTAT_H
# include <devstat.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct devstat.bytes_read,
					[AC_DEFINE(HAVE_DEVSTAT_BYTES_READ, 1, [struct devstat.bytes_read available])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_DEVSTAT_H
# include <devstat.h>
#endif
					]
			)])
		])

		AS_IF(	[test "x$ac_cv_type_struct_diskstats" = "xyes"], [
			# Disk IO read and write statistics are only present on recent OpenBSD.
			AC_CHECK_MEMBER(struct diskstats.ds_rbytes,
					[AC_DEFINE(HAVE_DISKSTAT_DS_RBYTES, , [New-style OpenBSD disk stats])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <sys/disk.h>
					]
			)])
			# Disk IO statistics have disknames only on recent OpenBSD.
			AC_CHECK_MEMBER(struct diskstats.ds_name,
					[AC_DEFINE(HAVE_DISKSTAT_DS_NAME, , [OpenBSD disk stats disk names])],
					[],
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <sys/disk.h>
					]
			)])
		])

		AS_IF(	[test "x$ac_cv_type_struct_kinfo_proc" = "xyes" -a "x$ac_cv_type_struct_kinfo_proc2" != "xyes"], [
			AC_CHECK_MEMBER(struct kinfo_proc.kp_proc,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_PROC, 1, [kp_proc member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.kp_thread,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_THREAD, 1, [kp_thread member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.kp_eproc,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_EPROC, 1, [kp_eproc member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.kp_eproc.e_ucred.cr_ruid,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_EPROC_E_UCRED_CR_RUID, 1, [kp_eproc.e_ucred.cr_ruid member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.ki_stat,
					[AC_DEFINE(HAVE_KINFO_PROC_KI_STAT, 1, [ki_stat member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.p_stat,
					[AC_DEFINE(HAVE_KINFO_PROC_P_STAT, 1, [p_stat member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.p_vm_map_size,
					[AC_DEFINE(HAVE_KINFO_PROC_P_VM_MAP_SIZE, 1, [p_vm_map_size member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.ki_structsize,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_THREAD, 1, [ki_structsize member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])
		])

			AC_CHECK_MEMBER(struct kinfo_proc.kp_pid,
					[AC_DEFINE(HAVE_KINFO_PROC_KP_PID, 1, [kp_pid member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

			AC_CHECK_MEMBER(struct kinfo_proc.p_pid,
					[AC_DEFINE(HAVE_KINFO_PROC_P_PID, 1, [p_pid member of struct kinfo_proc available])], ,
					[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
					]
			)])

		AC_CHECK_TYPES([struct uvmexp_sysctl], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_UVM_UVM_H
#include <uvm/uvm.h>
#endif
		])])

		AS_IF(	[test "x$ac_cv_type_struct_uvmexp_sysctl" != "xyes"], [
			AC_CHECK_TYPES([struct uvmexp], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_UVM_UVM_H
#include <uvm/uvm.h>
#endif
			])])

			AS_IF(	[test "x$ac_cv_type_struct_uvmexp" = "xyes"], [
				AC_CHECK_MEMBER(struct uvmexp.swtch,
						[AC_DEFINE(HAVE_STRUCT_UVMEXP_SWTCH, , [struct uvmexp.swtch])],
						[],
						[AC_LANG_SOURCE([
	AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_UVM_UVM_H
#include <uvm/uvm.h>
#endif
						]
				)])

				AC_CHECK_MEMBER(struct uvmexp.filepages,
						[AC_DEFINE(HAVE_STRUCT_UVMEXP_FILEPAGES, , [struct uvmexp.filepages])],
						[],
						[AC_LANG_SOURCE([
	AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_UVM_UVM_H
#include <uvm/uvm.h>
#endif
						]
				)])

				AC_CHECK_MEMBER(struct uvmexp.execpages,
						[AC_DEFINE(HAVE_STRUCT_UVMEXP_EXECPAGES, , [struct uvmexp.execpages])],
						[],
						[AC_LANG_SOURCE([
	AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_UVM_UVM_H
#include <uvm/uvm.h>
#endif
						]
				)])
			])
		])

		AC_CHECK_TYPES([struct vmtotal, struct vmmeter], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_VMMETER_H
#include <sys/vmmeter.h>
#endif
		])])

		AC_CHECK_TYPES([struct xswdev, struct xsw_usage], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_VM_VM_PARAM_H
#include <vm/vm_param.h>
#endif
		])])

		AS_IF(	[test "x$ac_cv_type_struct_xswdev" = "xyes"], [
			AC_CHECK_MEMBER([struct xswdev.xsw_size], [AC_DEFINE(HAVE_STRUCT_XSWDEV_SIZE, , [struct xswdev.xsw_size])], , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_VM_VM_PARAM_H
#include <vm/vm_param.h>
#endif
			])])
		])

		AC_CHECK_TYPES([struct xvfsconf, struct vfsconf], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
		])])
	]
)

AS_IF(	[test "x$ac_cv_func_getmntent" = "xyes" -o "x$ac_cv_func_getmntent_r" = "xyes"],
	[
		AC_ARG_WITH([mnttab], [
  --with-mnttab[[=/path/to/mtab]]
                          use (specified) mtab file
		], [WITH_MNTTAB="$withval"], [WITH_MNTTAB="check"])
		AC_CHECK_DECL(
			[MNT_MNTTAB], ,
			[
				AS_IF([test "x$cross_compiling" = "xyes"], , [
					AC_CHECK_FILE([/etc/mnttab], [AC_DEFINE_UNQUOTED([MNT_MNTTAB], "/etc/mnttab", [Configure determined mtab file])], [
						AC_CHECK_FILE([/etc/mtab], [AC_DEFINE_UNQUOTED([MNT_MNTTAB], "/etc/mtab", [Configure determined mtab file])])
					])
				])
			], [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif
			]
		)])

		AC_CHECK_DECL([setmntent], [AC_DEFINE([HAVE_DECL_SETMNTENT], 1, [define when setmntent() is declared])], ,[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif
			]
		)])

		AC_CHECK_DECL([endmntent], [AC_DEFINE([HAVE_DECL_ENDMNTENT], 1, [define when endmntent() is declared])], ,[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif
			]
		)])

		AC_CHECK_TYPES([struct mntent, struct mnttab], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif
			]
		)])

		AS_IF(	[test "x$ac_cv_func_getmntent" = "xyes"],
			[
				AC_CACHE_CHECK([apparent thread safety of getmntent], ac_cv_threadsafe_getmntent, [
					AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif

int main(void) {
	FILE *f;
#if defined(HAVE_STRUCT_MNTENT)
	struct mntent mp;
#elif defined(HAVE_STRUCT_MNTTAB)
	struct mnttab mp;
#endif
	int rc;

	rc = getmntent(f, &mp);

	return 0;
}
						])], [
							ac_cv_threadsafe_getmntent="yes"
						], [
							ac_cv_threadsafe_getmntent="no"
						]
					)
				])
				AS_IF(	[test "x$ac_cv_threadsafe_getmntent" = "xyes"],
					[AC_DEFINE([HAVE_THREADSAFE_GETMNTENT], 1, [define when getmntent takes pointer to storage to fill])]
				)
			]
		)
		AS_IF(	[test "x$ac_cv_func_getmntent_r" = "xyes"],
			[
				AC_MSG_CHECKING([return type of getmntent_r])
				AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif

int main(void) {
	FILE *f;
	struct mntent mp;
	char buf[[1024]];
	int rc;

	rc = getmntent_r(f, &mp, buf, sizeof(buf)) % 10;

	return 0;
}
					])], [
						AC_MSG_RESULT([int])
						AC_DEFINE([GETMNTENT_R_RETURN_INT], 1, [define when getmntent_r returns success code])
					], [
						AC_MSG_RESULT([struct mntent *])
					]
				)
			]
		)
	]
)

AC_CHECK_TYPES([struct statfs], , , [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
])])

AS_IF(	[test "x$ac_cv_type_struct_statfs" = "xyes"], [
		# check for several uncommon fields of struct statfs
		AC_CHECK_MEMBER(struct statfs.f_favail,
				[AC_DEFINE(HAVE_STATFS_FFAVAIL, , [define when statfs has f_favail member])],
				[],
				[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
				])])
		AC_CHECK_MEMBER(struct statfs.f_iosize,
				[AC_DEFINE(HAVE_STATFS_FIOSIZE, , [define when statfs has f_iosize member])],
				[],
				[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
				])])

		AC_CHECK_MEMBER(struct statfs.f_frsize,
				[AC_DEFINE(HAVE_STATFS_FFRSIZE, , [define when statfs has f_frsize member])],
				[],
				[AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
				])])
	]
)

dnl check whether procfs is really available
dnl AC_DEFINE(HAVE_PROCFS, 1, [Define to signal that procfs is available])

AX_CHECK_VISIBILITY(
[
	dnl AC_DEFINE_UNQUOTED([__sg_export], $ax_visibility_export, [Define export declarator])
	dnl AC_DEFINE_UNQUOTED([__sg_import], $ax_visibility_import, [Define import declarator])
	dnl AC_DEFINE_UNQUOTED([__sg_private], $ax_visibility_private, [Define private declarator])
	SG_EXPORT="$ax_visibility_export"
	SG_IMPORT="$ax_visibility_import"
	SG_PRIVATE="$ax_visibility_private"
	WITH_VISIBILITY_SUPPORT=1
	AC_SUBST(SG_EXPORT)
	AC_SUBST(SG_IMPORT)
	AC_SUBST(SG_PRIVATE)
],[
	dnl AC_DEFINE_UNQUOTED([__sg_export])
	dnl AC_DEFINE_UNQUOTED([__sg_import])
	dnl AC_DEFINE_UNQUOTED([__sg_private])
	WITH_VISIBILITY_SUPPORT=0
])
AC_SUBST(WITH_VISIBILITY_SUPPORT)

# User specified curses/ncurses prefix
AC_ARG_WITH([curses-prefix],
	[  --with-curses-prefix=PATH     curses (or ncurses) is in PATH],
	[
		if test -d "$withval/lib"; then
			SAIDARLIBS="-L${withval}/lib ${SAIDARLIBS}"
		fi
		if test -d "$withval/include"; then
			SAIDARCPPFLAGS="-I${withval}/include ${SAIRDARCPPFLAGS}"
		fi
	]
)

# Check if user wants to disable statgrab
AM_CONDITIONAL(STATGRAB, true)
AC_ARG_ENABLE(statgrab,
	[  --disable-statgrab      disable building of statgrab],
	[
		if test "x$enableval" = "xno" ; then
			AM_CONDITIONAL(STATGRAB, false)
		fi
	]
)

# Check if user wants to disable saidar
AM_CONDITIONAL(SAIDAR, true)
AC_ARG_ENABLE(saidar,
	[  --disable-saidar        disable building of saidar],
	[
		if test "x$enableval" = "xno" ; then
			AM_CONDITIONAL(SAIDAR, false)
		fi
	],
	[
		# saidar needs curses
		# (if we don't find curses we disable saidar)
		MP_WITH_CURSES
		AC_CHECK_HEADERS([termios.h sys/termios.h])
	]
)

# Check if the user would prefer not to build examples
AM_CONDITIONAL(EXAMPLES, true)
AC_ARG_ENABLE(examples,
	[  --disable-examples      disable building of examples],
	[
		if test "x$enableval" = "xno" ; then
			AM_CONDITIONAL(EXAMPLES, false)
		fi
	]
)

# Check if user wants to make and install manpages
# XXX split between make and install
AC_ARG_ENABLE(man-build,
[  --enable-man-build      enable building of manpages (default in maintainer-mode)
  --disable-man-build     disable building of manpages (default in all other cases)],
	[ENABLE_BUILD_MANPAGES="${enableval}"],
	[
		AS_IF(	[test "x$USE_MAINTAINER_MODE" = "xyes"],
			[ENABLE_BUILD_MANPAGES="yes"],
			[ENABLE_BUILD_MANPAGES="check"]
		)
	]
)

AM_CONDITIONAL(BUILD_MANPAGES, false)
AS_IF(	[test "x${ENABLE_MANPAGES}" = "xno"],
	[AM_CONDITIONAL(BUILD_MANPAGES, false)],
	[
		dnl check if whether docbook is available
		AC_MSG_CHECKING([for applicable docbook2man])
		AC_PATH_PROGS_FEATURE_CHECK([docbook2man_prog], [docbook2x-man docbook2man], [
			output=`$ac_path_docbook2man_prog --help 2>/dev/null | grep "Convert DocBook XML documents to man pages"`
			AS_IF([test -n "$output"], [
				ac_path_docbook2man_prog_found=:
				ac_cv_path_docbook2man_prog="$ac_path_docbook2man_prog"
			])
		], [
			docbook2man_prog="none"
			ac_cv_path_docbook2man_prog="none"
		])
		AC_MSG_RESULT($ac_cv_path_docbook2man_prog)
		AS_IF(	[test "x$enableval" = "xyes" -a "x$ac_cv_path_docbook2man_prog" = "xnone"],
			[
				AC_MSG_ERROR([Want manpages being build but have no docbook2man])
			],
			[test "x$ac_cv_path_docbook2man_prog" != "xnone"],
			[
				AM_CONDITIONAL(BUILD_MANPAGES, true)
				DOCBOOK2MAN="$ac_cv_path_docbook2man_prog"
				AC_SUBST(DOCBOOK2MAN)
				AC_MSG_NOTICE([enabling build of manpages using $DOCBOOK2MAN])
			]
		)
	]
)

# Check if user wants to make tests
AC_ARG_ENABLE(tests,
[  --enable-tests          enable test building (default in maintainer-mode)
  --disable-tests         disable test building (default in all other cases)],
	[ENABLE_TESTS="${enableval}"],
	[
		AS_IF(	[test "x$USE_MAINTAINER_MODE" = "xyes"],
			[ENABLE_TESTS="yes"],
			[ENABLE_TESTS="check"]
		)
	]
)

AM_CONDITIONAL(TESTS, false)
AM_CONDITIONAL(TEST_SCRIPTS, false)
AS_IF(	[test "x$ENABLE_TESTS" != "xno"],
	[
		AM_CONDITIONAL(TESTS, true)
		AX_PROG_PERL5( , , [5.006], [Getopt::Long, Test::More 0.90, Data::Dumper, IPC::Cmd], [
				AC_MSG_NOTICE([enabling build of test automation using $PERL5])
				AM_CONDITIONAL(TEST_SCRIPTS, true)
			], [
				AS_IF(	[test "x$ENABLE_TESTS" = "xyes"], [
					AC_MSG_FAILURE([Tests are required but no suitable Perl5 environment found])
				])
			]
		)
	]
)

# Check if the user wants to turn on setgid binaries
# If they specify nothing disable them
AC_ARG_ENABLE(setgid-binaries,
	[  --enable-setgid-binaries      enable binaries being setgid when needed],
	[
		if test "x$enableval" = "xno" ; then
			AM_CONDITIONAL(SETGIDBINS, false)
		fi
	],
	[
		AM_CONDITIONAL(SETGIDBINS, false)
	]

)

# Check if the user wants to turn on setuid binaries
# If they specify nothing disable them
AC_ARG_ENABLE(setuid-binaries,
	[  --enable-setuid-binaries      enable binaries being setuid when needed],
	[
		if test "x$enableval" = "xno" ; then
			AM_CONDITIONAL(SETUIDBINS, false)
		fi
	],
	[
		AM_CONDITIONAL(SETUIDBINS, false)
	]
)

# Check if the user wants to support access from multiple threads
AC_ARG_ENABLE(thread-support,
	[  --disable-thread-support      disable support for multi-threaded environments],
	[ENABLE_THREADS="$enableval"], [ENABLE_THREADS="$ENABLE_THREADS_DEFAULT"]
)

AS_IF([test "x$ac_cv_func_strerror_r" = "xyes"], [
	AC_MSG_CHECKING([return type of strerror_r])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
	AC_INCLUDES_DEFAULT

	int main(void) {
		char buf[[1024]];
		int rc;

		rc = strerror_r(1, buf, sizeof(buf)) % 10;

		return 0;
	}
		])], [
			AC_MSG_RESULT([int])
			AC_DEFINE([STRERROR_R_RETURN_INT], 1, [define when strerror_r returns success code])
		], [
			AC_MSG_RESULT([char *])
		]
	)
])

AM_CONDITIONAL(ENABLE_THREADS, false)
WITH_THREADS_CONFIGURED=0
AS_IF(	[test "x${ENABLE_THREADS}" != "xno"],
	[
		AX_WIN32(
		[
			AC_DEFINE([ENABLE_THREADS], 1, [Define if you want the thread support compiled in.])
			WITH_THREADS_CONFIGURED=1
			AM_CONDITIONAL(ENABLE_THREADS, true)
		],
		[
			AX_PTHREAD(
			[
				AC_DEFINE([ENABLE_THREADS], 1, [Define if you want the thread support compiled in.])
				AC_DEFINE(HAVE_PTHREAD, 1, [Define if you have POSIX threads libraries and header files.])
				CC="$PTHREAD_CC"
				CXX="$PTHREAD_CXX"
				AS_IF([test -n "$PTHREAD_LIBS"], [LINKFLAGS="$LINKFLAGS $PTHREAD_LIBS"])
				AS_IF([test -n "$PTHREAD_CFLAGS"], [CFLAGS="$CFLAGS $PTHREAD_CFLAGS"])
				AC_CHECK_HEADERS([pthread.h])
				AS_IF(	[test "x$ac_cv_func_strerror_r" != "xyes" -a "x$ac_cv_threadsafe_strerror" != "xyes"],
					[AC_MSG_ERROR([strerror_r or thread safe strerror required])])
				AM_CONDITIONAL(ENABLE_THREADS, true)
				WITH_THREADS_CONFIGURED=1

				# Braces checks derived from here:
				# http://git.savannah.gnu.org/gitweb/?p=guile.git;a=blob_plain;f=configure.ac;hb=HEAD
				old_CFLAGS="$CFLAGS"

				if test "$GCC" = "yes"; then
					# Since GCC only issues a warning for missing braces, so we need
					# `-Werror' to catch it.
					CFLAGS="-Werror -Wmissing-braces $CFLAGS"
				fi

				AC_CACHE_CHECK([whether PTHREAD_ONCE_INIT needs braces],
				  mp_cv_need_braces_on_pthread_once_init,
				  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>
				     pthread_once_t foo = PTHREAD_ONCE_INIT;]])],
				    [mp_cv_need_braces_on_pthread_once_init=no],
				    [mp_cv_need_braces_on_pthread_once_init=yes])])
				if test "$mp_cv_need_braces_on_pthread_once_init" = yes; then
					AC_DEFINE([NEED_PTHREAD_ONCE_INIT_BRACES], 1, [Define to 1 if PTHREAD_ONCE_INIT needs braces.])
				fi

				AC_CACHE_CHECK([whether PTHREAD_MUTEX_INITIALIZER needs braces],
				  mp_cv_need_braces_on_pthread_mutex_initializer,
				  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>
				     pthread_mutex_t foo = PTHREAD_MUTEX_INITIALIZER;]])],
				    [mp_cv_need_braces_on_pthread_mutex_initializer=no],
				    [mp_cv_need_braces_on_pthread_mutex_initializer=yes])])
				if test "$mp_cv_need_braces_on_pthread_mutex_initializer" = yes; then
					AC_DEFINE([NEED_PTHREAD_MUTEX_INITIALIZER_BRACES], 1, [Define to 1 if PTHREAD_ONCE_INIT needs braces.])
				fi

				CFLAGS="$old_CFLAGS"

				AC_SEARCH_LIBS([sem_init], [rt])
			],
			[
				AS_IF(	[test "x${ENABLE_THREADS}" = "xyes"],
					[AC_MSG_ERROR([No suitable threading library found])]
					[AC_MSG_NOTICE(No suitable threading library found - disable TLS support)])
			]
			)
		])
	]
) # endif : --enable-threads

AM_CONDITIONAL(WITH_LIBLOG4CPLUS, false)
AM_CONDITIONAL(WITH_FULL_CONSOLE_LOGGER, false)
WITH_LIBLOG4CPLUS=0

AC_ARG_ENABLE(logging,
	[  --enable-logging        enable logging in library (force console printf,
                          if no logging library is found)
  --disable-logging       disable logging in library], [ENABLE_LOGGING="$enableval"], [ENABLE_LOGGING="check"])

AS_IF([test "x$ENABLE_LOGGING" != "xno"], [
    AX_CHECK_WITH_LIB([log4cplus],[check])

    AX_CHECK_LIB_FLAGS([log4cplus], [stdc++,stdc++ unwind], [log4cplus_basic_configure();], [
    AC_INCLUDES_DEFAULT
#include <log4cplus/clogger.h>
      ], [log4cplus >= 1.0.5], [
	AM_CONDITIONAL(WITH_LIBLOG4CPLUS, true)
	WITH_LIBLOG4CPLUS=1
	AX_APPEND_TO_VAR([LINKFLAGS], ${LIBS_LOG4CPLUS})
	AX_APPEND_TO_VAR([CLIBFLAGS], ${INC_LOG4CPLUS})
	AC_DEFINE([WITH_LIBLOG4CPLUS], 1, [Define this when tracing shall be done using log4cplus])
	AC_MSG_NOTICE([compiling in support for liblog4cplus (LIBS="$LIBS_LOG4CPLUS}")])
      ]
    )

    AS_IF([test "x$WITH_LIBLOG4CPLUS" = "x1"], [
      AC_LANG_PUSH([C++])
      AC_MSG_CHECKING([if liblog4cplus is built with thread support])
      ax_cppflags_safe="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
      AC_COMPILE_IFELSE([AC_LANG_SOURCE([
      AC_INCLUDES_DEFAULT

#include <log4cplus/config.hxx>
#if defined(LOG4CPLUS_SINGLE_THREADED)
#error no threads
#endif

      int main() { return 0; }
	])], [
	  AC_MSG_RESULT([yes])
	  ax_liblog4cplus_threads=yes
	], [
	  AC_MSG_RESULT([no])
	  ax_liblog4cplus_threads=no
	]
      )
      CPPFLAGS="$ax_cppflags_safe"
      AC_LANG_POP([C++])
    ])

    AS_IF([test "x$ENABLE_LOGGING" = "xyes" -a "x$WITH_LIBLOG4CPLUS" != "x1"],
    [AM_CONDITIONAL(WITH_FULL_CONSOLE_LOGGER, false)
     AC_DEFINE([WITH_FULL_CONSOLE_LOGGER], 1, [Define this when tracing shall be done using fprintf(stderr, ...)])
     WITH_FULL_CONSOLE_LOGGER="1"])
])

AC_SUBST([WITH_LIBLOG4CPLUS])

dnl Libtool setup.
dnl must be latest to catch rpath compiled in by changes made during checks before

LT_INIT
AC_PROG_LIBTOOL

# The LIBS and CPPFLAGS that saidar needs
AC_SUBST(SAIDARLIBS)
AC_SUBST(SAIDARCPPFLAGS)

# The flags needed to link to libstatgrab
# (ie. our dependencies)
AC_SUBST(LINKFLAGS)
AC_SUBST(CLIBFLAGS)

# Ownership and permissions possibly needed for binaries
AC_SUBST(BIN_OWNER)
AC_SUBST(BIN_GROUP)
AC_SUBST(BIN_PERM)

dnl with or without threads - allow check in dependend libraries
AC_SUBST(WITH_THREADS_CONFIGURED)

# And finish by changing these files
AC_CONFIG_FILES([	Makefile
			src/Makefile src/libstatgrab/Makefile src/statgrab/Makefile src/saidar/Makefile
			docs/Makefile docs/libstatgrab/Makefile docs/statgrab/Makefile docs/saidar/Makefile
			tests/Makefile tests/testlib/Makefile tests/single_threaded/Makefile tests/multi_threaded/Makefile
			examples/Makefile
			libstatgrab.pc src/statgrab/statgrab-make-mrtg-config src/statgrab/statgrab-make-mrtg-index])

AC_CONFIG_FILES([src/libstatgrab/statgrab.h])
AC_OUTPUT
