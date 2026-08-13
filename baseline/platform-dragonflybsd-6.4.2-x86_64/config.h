/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Building on AIX 5.x - 9.x */
/* #undef AIX */

/* Building on some kind of BSD */
#define ALLBSD /**/

/* Building on Cygwin */
/* #undef CYGWIN */

/* Building on Darwin */
/* #undef DARWIN */

/* Building on DragonFlyBSD */
#define DFBSD /**/

/* Define if you want the thread support compiled in. */
/* #undef ENABLE_THREADS */

/* Define FMT string for gid_t */
#define FMT_GID_T "%u"

/* Define FMT string for pid_t */
#define FMT_PID_T "%d"

/* Define FMT string for size_t */
#define FMT_SIZE_T "%zu"

/* Define FMT string for ssize_t */
#define FMT_SSIZE_T "%zd"

/* Define FMT string for time_t */
#define FMT_TIME_T "%ld"

/* Define FMT string for uid_t */
#define FMT_UID_T "%u"

/* Building on FreeBSD */
/* #undef FREEBSD */

/* Building on FreeBSD 5+ */
/* #undef FREEBSD5 */

/* define when getmntent_r returns success code */
/* #undef GETMNTENT_R_RETURN_INT */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the `atoll' function. */
#define HAVE_ATOLL 1

/* Define to 1 if you have the `bzero' function. */
#define HAVE_BZERO 1

/* Define to 1 if you have the <ctype.h> header file. */
#define HAVE_CTYPE_H 1

/* Define to 1 if you have the <curses.h> header file. */
/* #undef HAVE_CURSES_H */

/* define when endmntent() is declared */
/* #undef HAVE_DECL_ENDMNTENT */

/* Define to 1 if you have the declaration of `endutent', and to 0 if you
   don't. */
/* #undef HAVE_DECL_ENDUTENT */

/* Define to 1 if you have the declaration of `endutxent', and to 0 if you
   don't. */
#define HAVE_DECL_ENDUTXENT 1

/* define when F_SETLK is declared */
#define HAVE_DECL_F_SETLK 1

/* define when getargs() is declared properly */
/* #undef HAVE_DECL_GETARGS */

/* define when getprocs64() is declared properly */
/* #undef HAVE_DECL_GETPROCS64 */

/* Define to 1 if you have the declaration of `getutent', and to 0 if you
   don't. */
/* #undef HAVE_DECL_GETUTENT */

/* Define to 1 if you have the declaration of `getutxent', and to 0 if you
   don't. */
#define HAVE_DECL_GETUTXENT 1

/* define when mntctl() is declared properly */
/* #undef HAVE_DECL_MNTCTL */

/* define when setmntent() is declared */
/* #undef HAVE_DECL_SETMNTENT */

/* Define to 1 if you have the declaration of `setutent', and to 0 if you
   don't. */
/* #undef HAVE_DECL_SETUTENT */

/* Define to 1 if you have the declaration of `setutxent', and to 0 if you
   don't. */
#define HAVE_DECL_SETUTXENT 1

/* Define to 1 if you have the declaration of `strlcat', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCAT 1

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCPY 1

/* struct devstat.bytes array available */
/* #undef HAVE_DEVSTAT_BYTES */

/* struct devstat.bytes_read available */
#define HAVE_DEVSTAT_BYTES_READ 1

/* Define to 1 if you have the `devstat_getdevs' function. */
/* #undef HAVE_DEVSTAT_GETDEVS */

/* Define to 1 if you have the <devstat.h> header file. */
#define HAVE_DEVSTAT_H 1

/* Define to 1 if you have the `devstat_selectdevs' function. */
/* #undef HAVE_DEVSTAT_SELECTDEVS */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* OpenBSD disk stats disk names */
/* #undef HAVE_DISKSTAT_DS_NAME */

/* New-style OpenBSD disk stats */
/* #undef HAVE_DISKSTAT_DS_RBYTES */

/* New-style NetBSD disk stats */
/* #undef HAVE_DISK_SYSCTL_DK_RBYTES */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `endutent' function. */
/* #undef HAVE_ENDUTENT */

/* Define to 1 if you have the `endutxent' function. */
#define HAVE_ENDUTXENT 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* struct ethtool_cmd.speed_hi available */
/* #undef HAVE_ETHTOOL_CMD_SPEED_HI */

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <float.h> header file. */
#define HAVE_FLOAT_H 1

/* Define to 1 if you have the `flock' function. */
#define HAVE_FLOCK 1

/* Define to 1 if you have the `getdevs' function. */
#define HAVE_GETDEVS 1

/* Define to 1 if you have the `getfsstat' function. */
#define HAVE_GETFSSTAT 1

/* Define to 1 if you have the `getloadavg' function. */
#define HAVE_GETLOADAVG 1

/* Define to 1 if you have the `getmntent' function. */
/* #undef HAVE_GETMNTENT */

/* Define to 1 if you have the `getmntent_r' function. */
/* #undef HAVE_GETMNTENT_R */

/* Define to 1 if you have the `getutent' function. */
/* #undef HAVE_GETUTENT */

/* Define to 1 if you have the `getutxent' function. */
#define HAVE_GETUTXENT 1

/* define when getvfsstat() is declared properly */
/* #undef HAVE_GETVFSSTAT */

/* Define to 1 if you have the `host_statistics' function. */
/* #undef HAVE_HOST_STATISTICS */

/* Define to 1 if you have the `host_statistics64' function. */
/* #undef HAVE_HOST_STATISTICS64 */

/* Define to 1 if you have the <ifaddrs.h> header file. */
#define HAVE_IFADDRS_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
/* #undef HAVE_IO_H */

/* struct io_sysctl.name available */
/* #undef HAVE_IO_SYSCTL_NAME */

/* Define to 1 if you have the `kinfo_get_sched_cputime' function. */
#define HAVE_KINFO_GET_SCHED_CPUTIME 1

/* Define to 1 if you have the <kinfo.h> header file. */
#define HAVE_KINFO_H 1

/* ki_stat member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_KI_STAT */

/* kp_eproc member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_KP_EPROC */

/* kp_eproc.e_ucred.cr_ruid member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_KP_EPROC_E_UCRED_CR_RUID */

/* kp_pid member of struct kinfo_proc available */
#define HAVE_KINFO_PROC_KP_PID 1

/* kp_proc member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_KP_PROC */

/* ki_structsize member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_KP_THREAD */

/* p_pid member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_P_PID */

/* p_stat member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_P_STAT */

/* p_vm_map_size member of struct kinfo_proc available */
/* #undef HAVE_KINFO_PROC_P_VM_MAP_SIZE */

/* Define to 1 if you have the <kstat.h> header file. */
/* #undef HAVE_KSTAT_H */

/* Define to 1 if you have the <libdevinfo.h> header file. */
/* #undef HAVE_LIBDEVINFO_H */

/* Define to 1 if you have the <libgen.h> header file. */
#define HAVE_LIBGEN_H 1

/* Define to 1 if you have the <libperfstat.h> header file. */
/* #undef HAVE_LIBPERFSTAT_H */

/* Define to 1 if you have the <libproc.h> header file. */
/* #undef HAVE_LIBPROC_H */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <linux/ethtool.h> header file. */
/* #undef HAVE_LINUX_ETHTOOL_H */

/* Define to 1 if you have the <linux/sockios.h> header file. */
/* #undef HAVE_LINUX_SOCKIOS_H */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `log4cplus_add_log_level' function. */
/* #undef HAVE_LOG4CPLUS_ADD_LOG_LEVEL */

/* Define to 1 if you have the `log4cplus_initialize' function. */
/* #undef HAVE_LOG4CPLUS_INITIALIZE */

/* Define to 1 if the system has the type `long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <mach/host_info.h> header file. */
/* #undef HAVE_MACH_HOST_INFO_H */

/* Define to 1 if you have the <mach/kern_return.h> header file. */
/* #undef HAVE_MACH_KERN_RETURN_H */

/* Define to 1 if you have the <mach/machine.h> header file. */
/* #undef HAVE_MACH_MACHINE_H */

/* Define to 1 if you have the <mach/mach.h> header file. */
/* #undef HAVE_MACH_MACH_H */

/* Define to 1 if you have the <mach/mach_host.h> header file. */
/* #undef HAVE_MACH_MACH_HOST_H */

/* Define to 1 if you have the <mach/vm_statistics.h> header file. */
/* #undef HAVE_MACH_VM_STATISTICS_H */

/* Define to 1 if you have the `malloc' function. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the `mntctl' function. */
/* #undef HAVE_MNTCTL */

/* Define to 1 if you have the <mntent.h> header file. */
/* #undef HAVE_MNTENT_H */

/* Define to 1 if you have the <ncurses.h> header file. */
/* #undef HAVE_NCURSES_H */

/* Define to 1 if you have the <ncurses/ncurses.h> header file. */
#define HAVE_NCURSES_NCURSES_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/if_ether.h> header file. */
#define HAVE_NETINET_IF_ETHER_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the <net/if_media.h> header file. */
#define HAVE_NET_IF_MEDIA_H 1

/* define this when array at struct end might be open */
#define HAVE_OPEN_ARRAY_AT_STRUCT_END 1

/* Define to 1 if you have the <paths.h> header file. */
#define HAVE_PATHS_H 1

/* Define to 1 if you have the <pdh.h> header file. */
/* #undef HAVE_PDH_H */

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define to signal that procfs is available */
/* #undef HAVE_PROCFS */

/* Define to 1 if you have the <procfs.h> header file. */
/* #undef HAVE_PROCFS_H */

/* Define to 1 if you have the <procinfo.h> header file. */
/* #undef HAVE_PROCINFO_H */

/* Define to 1 if you have the `proc_pidinfo' function. */
/* #undef HAVE_PROC_PIDINFO */

/* pst_diskinfo.psd_dkbytewrite */
/* #undef HAVE_PST_DISKINFO_PSD_DKBYTEWRITE */

/* Define if you have POSIX threads libraries and header files. */
/* #undef HAVE_PTHREAD */

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef HAVE_PTHREAD_H */

/* Have PTHREAD_PRIO_INHERIT. */
/* #undef HAVE_PTHREAD_PRIO_INHERIT */

/* Define to 1 if you have the `realloc' function. */
#define HAVE_REALLOC 1

/* Define to 1 if you have the <regex.h> header file. */
#define HAVE_REGEX_H 1

/* Define to 1 if you have the <sched.h> header file. */
#define HAVE_SCHED_H 1

/* Define to 1 if you have the `selectdevs' function. */
#define HAVE_SELECTDEVS 1

/* Define to 1 if you have the `setegid' function. */
#define HAVE_SETEGID 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Define to 1 if you have the <setjmp.h> header file. */
#define HAVE_SETJMP_H 1

/* Define to 1 if you have the `setlinebuf' function. */
/* #undef HAVE_SETLINEBUF */

/* Define to 1 if you have the `setresgid' function. */
#define HAVE_SETRESGID 1

/* Define to 1 if you have the `setresuid' function. */
#define HAVE_SETRESUID 1

/* Define to 1 if you have the `setutent' function. */
/* #undef HAVE_SETUTENT */

/* Define to 1 if you have the `setutxent' function. */
#define HAVE_SETUTXENT 1

/* Define to 1 if you have the `setvbuf' function. */
#define HAVE_SETVBUF 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `statfs' function. */
#define HAVE_STATFS 1

/* define when statfs has f_favail member */
/* #undef HAVE_STATFS_FFAVAIL */

/* define when statfs has f_frsize member */
/* #undef HAVE_STATFS_FFRSIZE */

/* define when statfs has f_iosize member */
#define HAVE_STATFS_FIOSIZE /**/

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define to 1 if you have the `statvfs64' function. */
/* #undef HAVE_STATVFS64 */

/* Solaris etc. extended statvfs64 */
/* #undef HAVE_STATVFS64_FBASETYPE */

/* another Solaris extension */
/* #undef HAVE_STATVFS64_FFRSIZE */

/* another BSD extension */
/* #undef HAVE_STATVFS64_FIOSIZE */

/* NetBSD extended statvfs64 */
/* #undef HAVE_STATVFS64_FSTYPENAME */

/* Solaris etc. extended statvfs */
/* #undef HAVE_STATVFS_FBASETYPE */

/* another Solaris extension */
#define HAVE_STATVFS_FFRSIZE /**/

/* another BSD extension */
/* #undef HAVE_STATVFS_FIOSIZE */

/* NetBSD extended statvfs */
/* #undef HAVE_STATVFS_FSTYPENAME */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror_r' function. */
#define HAVE_STRERROR_R 1

/* Define to 1 if you have the `strerror_s' function. */
/* #undef HAVE_STRERROR_S */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
#define HAVE_STRLCAT 1

/* Define to 1 if you have the `strlcpy' function. */
#define HAVE_STRLCPY 1

/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY 1

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if the system has the type `struct anoninfo'. */
/* #undef HAVE_STRUCT_ANONINFO */

/* Define to 1 if the system has the type `struct devstat'. */
#define HAVE_STRUCT_DEVSTAT 1

/* Define to 1 if the system has the type `struct diskstats'. */
/* #undef HAVE_STRUCT_DISKSTATS */

/* Define to 1 if the system has the type `struct disk_sysctl'. */
/* #undef HAVE_STRUCT_DISK_SYSCTL */

/* Define to 1 if the system has the type `struct io_sysctl'. */
/* #undef HAVE_STRUCT_IO_SYSCTL */

/* Define to 1 if the system has the type `struct kinfo_cputime'. */
#define HAVE_STRUCT_KINFO_CPUTIME 1

/* Define to 1 if the system has the type `struct kinfo_proc'. */
#define HAVE_STRUCT_KINFO_PROC 1

/* Define to 1 if the system has the type `struct kinfo_proc2'. */
/* #undef HAVE_STRUCT_KINFO_PROC2 */

/* Define to 1 if the system has the type `struct mntent'. */
/* #undef HAVE_STRUCT_MNTENT */

/* Define to 1 if the system has the type `struct mnttab'. */
/* #undef HAVE_STRUCT_MNTTAB */

/* Define to 1 if the system has the type `struct proc_bsdinfo'. */
/* #undef HAVE_STRUCT_PROC_BSDINFO */

/* Define to 1 if the system has the type `struct proc_taskinfo'. */
/* #undef HAVE_STRUCT_PROC_TASKINFO */

/* Define to 1 if `ss_family' is a member of `struct sockaddr_storage'. */
#define HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY 1

/* Define to 1 if `__ss_family' is a member of `struct sockaddr_storage'. */
/* #undef HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY */

/* Define to 1 if the system has the type `struct statfs'. */
#define HAVE_STRUCT_STATFS 1

/* Define to 1 if the system has the type `struct statinfo'. */
#define HAVE_STRUCT_STATINFO 1

/* Define to 1 if the system has the type `struct statvfs'. */
#define HAVE_STRUCT_STATVFS 1

/* Define to 1 if the system has the type `struct statvfs64'. */
/* #undef HAVE_STRUCT_STATVFS64 */

/* Define to 1 if the system has the type `struct swapent'. */
/* #undef HAVE_STRUCT_SWAPENT */

/* Define to 1 if the system has the type `struct swaptable'. */
/* #undef HAVE_STRUCT_SWAPTABLE */

/* Define to 1 if the system has the type `struct utmp'. */
/* #undef HAVE_STRUCT_UTMP */

/* Define to 1 if the system has the type `struct utmpx'. */
#define HAVE_STRUCT_UTMPX 1

/* Define to 1 if the system has the type `struct uvmexp'. */
/* #undef HAVE_STRUCT_UVMEXP */

/* struct uvmexp.execpages */
/* #undef HAVE_STRUCT_UVMEXP_EXECPAGES */

/* struct uvmexp.filepages */
/* #undef HAVE_STRUCT_UVMEXP_FILEPAGES */

/* struct uvmexp.swtch */
/* #undef HAVE_STRUCT_UVMEXP_SWTCH */

/* Define to 1 if the system has the type `struct uvmexp_sysctl'. */
/* #undef HAVE_STRUCT_UVMEXP_SYSCTL */

/* Define to 1 if the system has the type `struct vfsconf'. */
#define HAVE_STRUCT_VFSCONF 1

/* Define to 1 if the system has the type `struct vmmeter'. */
#define HAVE_STRUCT_VMMETER 1

/* Define to 1 if the system has the type `struct vmtotal'. */
#define HAVE_STRUCT_VMTOTAL 1

/* Define to 1 if the system has the type `struct xswdev'. */
#define HAVE_STRUCT_XSWDEV 1

/* struct xswdev.xsw_size */
#define HAVE_STRUCT_XSWDEV_SIZE /**/

/* Define to 1 if the system has the type `struct xsw_usage'. */
/* #undef HAVE_STRUCT_XSW_USAGE */

/* Define to 1 if the system has the type `struct xvfsconf'. */
/* #undef HAVE_STRUCT_XVFSCONF */

/* Define to 1 if you have the `sysctlbyname' function. */
#define HAVE_SYSCTLBYNAME 1

/* Define to 1 if you have the `sysfs' function. */
/* #undef HAVE_SYSFS */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/disk.h> header file. */
/* #undef HAVE_SYS_DISK_H */

/* Define to 1 if you have the <sys/dkstat.h> header file. */
/* #undef HAVE_SYS_DKSTAT_H */

/* Define to 1 if you have the <sys/dk.h> header file. */
/* #undef HAVE_SYS_DK_H */

/* Define to 1 if you have the <sys/dlpi_ext.h> header file. */
/* #undef HAVE_SYS_DLPI_EXT_H */

/* Define to 1 if you have the <sys/dlpi.h> header file. */
/* #undef HAVE_SYS_DLPI_H */

/* Define to 1 if you have the <sys/dr.h> header file. */
/* #undef HAVE_SYS_DR_H */

/* Define to 1 if you have the <sys/fstyp.h> header file. */
/* #undef HAVE_SYS_FSTYP_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/iostat.h> header file. */
/* #undef HAVE_SYS_IOSTAT_H */

/* Define to 1 if you have the <sys/loadavg.h> header file. */
/* #undef HAVE_SYS_LOADAVG_H */

/* Define to 1 if you have the <sys/mib.h> header file. */
/* #undef HAVE_SYS_MIB_H */

/* Define to 1 if you have the <sys/mntctl.h> header file. */
/* #undef HAVE_SYS_MNTCTL_H */

/* Define to 1 if you have the <sys/mnttab.h> header file. */
/* #undef HAVE_SYS_MNTTAB_H */

/* Define to 1 if you have the <sys/mnttent.h> header file. */
/* #undef HAVE_SYS_MNTTENT_H */

/* Define to 1 if you have the <sys/mount.h> header file. */
#define HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/proc.h> header file. */
/* #undef HAVE_SYS_PROC_H */

/* Define to 1 if you have the <sys/protosw.h> header file. */
/* #undef HAVE_SYS_PROTOSW_H */

/* Define to 1 if you have the <sys/pstat.h> header file. */
/* #undef HAVE_SYS_PSTAT_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/sar.h> header file. */
/* #undef HAVE_SYS_SAR_H */

/* Define to 1 if you have the <sys/sched.h> header file. */
#define HAVE_SYS_SCHED_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/statfs.h> header file. */
/* #undef HAVE_SYS_STATFS_H */

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/stropts.h> header file. */
/* #undef HAVE_SYS_STROPTS_H */

/* Define to 1 if you have the <sys/swap.h> header file. */
/* #undef HAVE_SYS_SWAP_H */

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#define HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/sysinfo.h> header file. */
/* #undef HAVE_SYS_SYSINFO_H */

/* Define to 1 if you have the <sys/systeminfo.h> header file. */
/* #undef HAVE_SYS_SYSTEMINFO_H */

/* Define to 1 if you have the <sys/termios.h> header file. */
/* #undef HAVE_SYS_TERMIOS_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/ucred.h> header file. */
#define HAVE_SYS_UCRED_H 1

/* Define to 1 if you have the <sys/unistd.h> header file. */
#define HAVE_SYS_UNISTD_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/user.h> header file. */
#define HAVE_SYS_USER_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <sys/vfs.h> header file. */
/* #undef HAVE_SYS_VFS_H */

/* Define to 1 if you have the <sys/vmmeter.h> header file. */
#define HAVE_SYS_VMMETER_H 1

/* Define to 1 if you have the <sys/vmount.h> header file. */
/* #undef HAVE_SYS_VMOUNT_H */

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* define when getmntent takes pointer to storage to fill */
/* #undef HAVE_THREADSAFE_GETMNTENT */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the system has the type `unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* utmp */
/* #undef HAVE_UTMP */

/* utmpx */
#define HAVE_UTMPX /**/

/* Define to 1 if you have the <utmpx.h> header file. */
#define HAVE_UTMPX_H 1

/* utmpx.ut_host */
#define HAVE_UTMPX_HOST /**/

/* utmpx.ut_syslen */
/* #undef HAVE_UTMPX_SYSLEN */

/* Define to 1 if you have the <utmp.h> header file. */
/* #undef HAVE_UTMP_H */

/* utmp.ut_host */
/* #undef HAVE_UTMP_HOST */

/* utmp.ut_id */
/* #undef HAVE_UTMP_ID */

/* utmp.ut_name */
/* #undef HAVE_UTMP_NAME */

/* utmp.ut_pid */
/* #undef HAVE_UTMP_PID */

/* utmp.ut_time */
/* #undef HAVE_UTMP_TIME */

/* utmp.ut_type */
/* #undef HAVE_UTMP_TYPE */

/* utmp.ut_user */
/* #undef HAVE_UTMP_USER */

/* Define to 1 if you have the <uvm/uvm.h> header file. */
/* #undef HAVE_UVM_UVM_H */

/* Define to 1 if the system has the type `vaddr_t'. */
/* #undef HAVE_VADDR_T */

/* v_cache_count member of struct vmmeter available */
/* #undef HAVE_VMMETER_V_CACHE_COUNT */

/* Define to 1 if you have the <vm/vm_param.h> header file. */
#define HAVE_VM_VM_PARAM_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsprintf' function. */
#define HAVE_VSPRINTF 1

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the <winsock.h> header file. */
/* #undef HAVE_WINSOCK_H */

/* Define to 1 if you have the <ws2tcpip.h> header file. */
/* #undef HAVE_WS2TCPIP_H */

/* Define to 1 if you have the <wspiapi.h> header file. */
/* #undef HAVE_WSPIAPI_H */

/* _XOPEN_SOURCE $HAVE_XOPEN_SOURCE */
#define HAVE_XOPEN_SOURCE 600

/* define this when array can have zero length */
#define HAVE_ZERO_SIZED_ARRAY 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Building on HP-UX 11.11+ */
/* #undef HPUX */

/* Building on HP-UX 11.11+ */
/* #undef HPUX11 */

/* Building on GNU/Linux */
/* #undef LINUX */

/* define when linux/ethtool.h has broken types */
/* #undef LINUX_BROKEN_ETHTOOL_TYPES */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Building on MinGW */
/* #undef MINGW */

/* Configure specified mtab file */
/* #undef MNT_MNTTAB */

/* Define to 1 if PTHREAD_ONCE_INIT needs braces. */
/* #undef NEED_PTHREAD_MUTEX_INITIALIZER_BRACES */

/* Define to 1 if PTHREAD_ONCE_INIT needs braces. */
/* #undef NEED_PTHREAD_ONCE_INIT_BRACES */

/* Building on NetBSD */
/* #undef NETBSD */

/* Building on OpenBSD */
/* #undef OPENBSD */

/* Define to be the name of the operating system. */
#define OS_TYPE "dragonfly6.4"

/* Name of package */
#define PACKAGE "libstatgrab"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://libstatgrab.org/issues"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libstatgrab"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libstatgrab 0.92.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libstatgrab"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://libstatgrab.org/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.92.1"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* The size of `utmpx.ut_id', as computed by sizeof. */
#define SIZEOF_UTMPX_UT_ID 0

/* The size of `utmp.ut_id', as computed by sizeof. */
/* #undef SIZEOF_UTMP_UT_ID */

/* Building on Solaris */
/* #undef SOLARIS */

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* define when strerror_r returns success code */
#define STRERROR_R_RETURN_INT 1

/* Defined with a true value when struct sg_vector is well aligned */
#define STRUCT_SG_VECTOR_ALIGN_OK 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. This
   macro is obsolete. */
#define TIME_WITH_SYS_TIME 1

/* Version number of package */
#define VERSION "0.92.1"

/* Building for Windows 2003 */
/* #undef WINVER */

/* Define this when tracing shall be done using fprintf(stderr, ...) */
/* #undef WITH_FULL_CONSOLE_LOGGER */

/* Define this when tracing shall be done using log4cplus */
/* #undef WITH_LIBLOG4CPLUS */

/* _KMEMUSER needed on NetBSD for vaddr_t */
/* #undef _KMEMUSER */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* struct vmmeter needs to be pleased */
/* #undef _WANT_VMMETER */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define as a signed integer type capable of holding a process identifier. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef time_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */
