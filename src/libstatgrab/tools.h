/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 * $Id$
 */

#ifndef STATGRAB_TOOLS_H
#define STATGRAB_TOOLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
# include <assert.h>
# include <ctype.h>
# include <errno.h>
# include <float.h>
# include <limits.h>
# include <locale.h>
# include <math.h>
# include <setjmp.h>
# include <signal.h>
# include <stdarg.h>
# include <stddef.h>
# include <stdlib.h>
# include <string.h>
#else
# ifdef HAVE_FLOAT_H
#  include <float.h>
# endif
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# ifdef HAVE_MATH_H
#  include <math.h>
# endif
# ifdef HAVE_SETJMP_H
#  include <setjmp.h>
# endif
# ifdef HAVE_STDARG_H
#  include <stdarg.h>
# endif
# ifdef HAVE_CTYPE_H
#  include <ctype.h>
# endif
# ifdef HAVE_SIGNAL_H
#  include <signal.h>
# endif
# ifdef HAVE_ERRNO_H
#  include <errno.h>
# endif
# ifdef HAVE_LIMITS_H
#  include <limits.h>
# endif
# ifdef HAVE_ASSERT_H
#  include <assert.h>
# endif
# ifdef HAVE_STDDEF_H
#  include <stddef.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#elif defined(HAVE_SYS_UNISTD_H)
# include <sys/unistd.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen ((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) ((dirent)->d_namlen)
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#ifdef _WIN32
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
#  if HAVE_WS2TCPIP_H
#   include <ws2tcpip.h>
#  endif
#  if HAVE_WSPIAPI_H
#   include <wspiapi.h>
#  endif
# elif defined(HAVE_WINSOCK_H)
  /* IIRC winsock.h must be included before windows.h */
#  include <winsock.h>
# endif

# ifdef HAVE_IO_H
#  include <io.h>
# endif
# ifdef HAVE_PSAPI_H
#  include <psapi.h>
# endif
# ifdef HAVE_PROCESS_H
#  include <process.h>
# endif
# include <windows.h>
# ifdef HAVE_PDH_H
#  include <pdh.h>
# endif
# ifdef HAVE_IPHLPAPI_H
#  include <Iphlpapi.h>
# endif
# ifdef HAVE_LM_H
#  include <lm.h>
# endif
# include "win32.h"
#else
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
# ifdef HAVE_NET_IF_H
#  include <net/if.h>
# endif
#endif

#if 0
#ifndef HAVE_STRCASECMP
# ifdef HAVESTRICMP
#  define strcasecmp stricmp
# else
extern "C" strcasecmp(const char *s1, const char *s2);
# endif
#endif
#endif

#ifndef HAVE_GETPID
# ifdef HAVE__GETPID
#  define getpid _getpid
# endif
#endif

#ifndef HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY
# ifdef HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY
#  define ss_family __ss_family
# endif
#endif

#ifdef HAVE_PTHREAD_H
# include <pthread.h>
#endif

#if HAVE_REGEX_H
# include <regex.h>
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif
#ifdef HAVE_SYS_PROC_H
# include <sys/proc.h>
#endif
#ifdef HAVE_UVM_UVM_H
# include <uvm/uvm.h>
#endif
#ifdef HAVE_SYS_VMMETER_H
#include <sys/vmmeter.h>
#endif
#ifdef HAVE_SYS_PSTAT_H
# include <sys/pstat.h>
#endif
#ifdef HAVE_SYS_SAR_H
# include <sys/sar.h>
#endif
#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif
#ifdef HAVE_SYS_SYSCTL_H
# include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_LOADAVG_H
# include <sys/loadavg.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#endif
#ifdef HAVE_KSTAT_H
# include <kstat.h>
#endif
#ifdef HAVE_PROCFS_H
# include <procfs.h>
#endif
#ifdef HAVE_SYS_DR_H
# include <sys/dr.h>
#endif
#ifdef HAVE_SYS_PROTOSW_H
# include <sys/protosw.h>
#endif
#ifdef HAVE_LIBPERFSTAT_H
# include <libperfstat.h>
#endif
#ifdef HAVE_PROCINFO_H
# include <procinfo.h>
#endif
#ifdef HAVE_SYS_DK_H
# include <sys/dk.h>
#endif
#ifdef HAVE_SYS_DKSTAT_H
# include <sys/dkstat.h>
#endif
#ifdef HAVE_SCHED_H
# include <sched.h>
#endif
#ifdef HAVE_SYS_SCHED_H
# include <sys/sched.h>
#endif
#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif
#ifdef HAVE_SYS_MNTENT_H
# include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_UCRED_H
# include <sys/ucred.h>
#endif
#ifdef HAVE_VM_VM_PARAM_H
# include <vm/vm_param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
# include <sys/mount.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#ifdef HAVE_DEVSTAT_H
# include <devstat.h>
#endif
#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif
#ifdef HAVE_SYS_IOSTAT_H
#include <sys/iostat.h>
#endif
#ifdef HAVE_FSTAB_H
# include <fstab.h>
#endif
#ifdef HAVE_SYS_STATFS_H
# include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_MNTCTL_H
# include <sys/mntctl.h>
#endif
#ifdef HAVE_SYS_VMOUNT_H
# include <sys/vmount.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
# include <sys/sockio.h>
#endif
#ifdef HAVE_LINUX_ETHTOOL_H
# include <linux/ethtool.h>
#endif
#ifdef HAVE_LINUX_SOCKIOS_H
# include <linux/sockios.h>
#endif
#ifdef HAVE_IFADDRS_H
# include <ifaddrs.h>
#endif
#ifdef HAVE_NET_IF_MEDIA_H
# include <net/if_media.h>
#endif
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif
#ifdef HAVE_SYS_STROPTS_H
# include <sys/stropts.h>
#endif
#ifdef HAVE_SYS_DLPI_H
# include <sys/dlpi.h>
#endif
#ifdef HAVE_SYS_DLPI_EXT_H
# include <sys/dlpi_ext.h>
#endif
#ifdef HAVE_SYS_MIB_H
# include <sys/mib.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif
#ifdef HAVE_SYS_SWAP_H
# include <sys/swap.h>
#endif
#ifdef HAVE_SYS_FSTYP_H
# include <sys/fstyp.h>
#endif
#if defined(HAVE_UTMPX_H)
# include <utmpx.h>
#elif defined(HAVE_UTMP_H)
# include <utmp.h>
#endif
#ifdef HAVE_LIBDEVINFO_H
# include <libdevinfo.h>
#endif
#ifdef HAVE_MACH_MACH_H
# include <mach/mach.h>
#else
# ifdef HAVE_MACH_MACHINE_H
#  include <mach/machine.h>
# endif
# ifdef HAVE_MACH_MACH_HOST_H
#  include <mach/mach_host.h>
# endif
# ifdef HAVE_MACH_VM_STATISTICS_H
#  include <mach/vm_statistics.h>
# endif
# ifdef HAVE_MACH_KERN_RETURN_H
#  include <mach/kern_return.h>
# endif
# ifdef HAVE_MACH_HOST_INFO_H
#  include <mach/host_info.h>
# endif
#endif
#ifdef HAVE_KINFO_H
# include <kinfo.h>
#endif
#ifdef HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif

#include "statgrab.h"

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#endif

#if defined(__ICC) && defined(offsetof)
# undef offsetof
#endif

#ifndef offsetof
# ifdef __ICC
#  define offsetof(type,memb) ((size_t)(((char *)(&((type*)0)->memb)) - ((char *)0)))
# else
#  define offsetof(type,memb) ((size_t)(((char *)(&((type*)0)->memb)) - ((char *)0)))
# endif
#endif

#define MAGIC_EYE(a,b,c,d) (((unsigned)(a) & 0xFF) + (((unsigned)(b) & 0xFF) << 8) + (((unsigned)(c) & 0xFF) << 16) + (((unsigned)(d) & 0xFF) << 24))

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#include "trace.h"

#ifdef SOLARIS
__sg_private const char *sg_get_svr_from_bsd(const char *bsd);
#endif

#if !defined(HAVE_FLOCK) && defined(HAVE_FCNTL) && defined(HAVE_DECL_F_SETLK)
__sg_private int flock(int fd, int op);
# ifndef LOCK_SH
#  define LOCK_SH 1
# endif
# ifndef LOCK_EX
#  define LOCK_EX 2
# endif
# ifndef LOCK_UN
#  define LOCK_UN 3
# endif
# ifndef LOCK_NB
#  define LOCK_NB 8
# endif
#endif

#if (defined(HAVE_GETMNTENT) || defined(HAVE_GETMNTENT_R) )
#  if !defined(HAVE_DECL_SETMNTENT) || !defined(HAVE_DECL_ENDMNTENT)
__sg_private FILE *setmntent(const char *fn, const char *type);
__sg_private int endmntent(FILE *f);
#  endif
#endif

#ifndef HAVE_STRLCAT
__sg_export size_t sg_strlcat(char *dst, const char *src, size_t siz);
#else
#define sg_strlcat strlcat
#endif

#ifndef HAVE_STRLCPY
__sg_export size_t sg_strlcpy(char *dst, const char *src, size_t siz);
#else
#define sg_strlcpy strlcpy
#endif

#include "error.h"
#include "vector.h"
#include "globals.h"

__sg_private sg_error sg_update_string(char **dest, const char *src);
__sg_private sg_error sg_lupdate_string(char **dest, const char *src, size_t maxlen);
__sg_private sg_error sg_update_mem(void **dest, const void *src, size_t len);
__sg_private sg_error sg_concat_string(char **dest, const char *src);

#if defined(LINUX) || defined(CYGWIN)
__sg_private long long sg_get_ll_match(char *line, regmatch_t *match);
__sg_private char *sg_get_string_match(char *line, regmatch_t *match);

__sg_private char *sg_f_read_line(FILE *f, char *linebuf, size_t buf_size, const char *string);
#endif

#if defined(__NEED_SG_GET_SYS_PAGE_SIZE)
static ssize_t sys_page_size;

static inline ssize_t
sg_get_sys_page_size(void) {
	if( 0 == sys_page_size ) {
		if( ( sys_page_size = sysconf(_SC_PAGESIZE) ) == -1 ) {
			SET_ERROR_WITH_ERRNO("tools", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
		}
	}

	return sys_page_size;
}
#endif

__sg_private void *sg_realloc(void *ptr, size_t size);
#define sg_malloc(size) sg_realloc(NULL, size)

#define BITS_PER_ITEM(vect) \
	(8*sizeof((vect)[0]))
#define BIT_SET(vect,bit) \
	(vect)[ (bit) / BITS_PER_ITEM(vect) ] |= ( 1 << ( (bit) % BITS_PER_ITEM(vect) ) )
#define BIT_ISSET(vect,bit) \
	( 0 != ( (vect)[ (bit) / BITS_PER_ITEM(vect) ] & ( 1 << ( (bit) % BITS_PER_ITEM(vect) ) ) ) )
#define BIT_GET(vect,bit) \
	( ( (vect)[ (bit) / BITS_PER_ITEM(vect) ] & ( 1 << ( (bit) % BITS_PER_ITEM(vect) ) ) ) >> ( (bit) % BITS_PER_ITEM(vect) ) )
#define BIT_SCAN_FWD(vect,bit) \
	while( 0 == BIT_ISSET(vect,bit) ) ++(bit)

#endif /* ?STATGRAB_TOOLS_H */
