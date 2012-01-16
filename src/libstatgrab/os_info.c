/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

#include "tools.h"

/**
 * Note: values must be sorted, search will be done using bsearch
 */
#if defined(SOLARIS)
#define GET_BITWIDTH_BY_ARCH_NAME
static const char *isa32[] = { "i386", "sparc" };
static const char *isa64[] = { "amd64", "sparcv9" };
#elif defined(CTL_HW) && defined(HW_MACHINE_ARCH)
#define GET_BITWIDTH_BY_ARCH_NAME
static const char *isa32[] = { "arm", "armeb", "hppa", "i386", "m68000", "m68k", "mips", "powerpc", "sh3el", "sh3eb", "sparc", "vax" };
static const char *isa64[] = { "alpha", "amd64", "hppa64", "ia64", "mips64", "mips64el", "sparc64", "x86_64" };
#endif

#ifdef WIN32
#define WINDOWS2000 "Windows 2000"
#define WINDOWSXP "Windows XP"
#define WINDOWS2003 "Windows Server 2003"
#define BUFSIZE 12

static int
home_or_pro(const OSVERSIONINFOEX osinfo, char **name)
{
	int r;

	if (osinfo.wSuiteMask & VER_SUITE_PERSONAL) {
		r = sg_concat_string(name, " Home Edition");
	} else {
		r = sg_concat_string(name, " Professional");
	}
	return r;
}

static char *
get_os_name(const OSVERSIONINFOEX osinfo)
{
	char *name;
	char tmp[10];
	int r = 0;

	/* we only compile on 2k or newer, which is version 5
	 * Covers 2000, XP and 2003 */
	if (osinfo.dwMajorVersion != 5) {
		return "Unknown";
	}
	switch(osinfo.dwMinorVersion) {
		case 0: /* Windows 2000 */
			name = strdup(WINDOWS2000);
			if(name == NULL) {
				goto out;
			}
			if (osinfo.wProductType == VER_NT_WORKSTATION) {
				r = home_or_pro(osinfo, &name);
			} else if (osinfo.wSuiteMask & VER_SUITE_DATACENTER) {
				r = sg_concat_string(&name, " Datacenter Server");
			} else if (osinfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
				r = sg_concat_string(&name, " Advanced Server");
			} else {
				r = sg_concat_string(&name, " Server");
			}
			break;
		case 1: /* Windows XP */
			name = strdup(WINDOWSXP);
			if(name == NULL) {
				goto out;
			}
			r = home_or_pro(osinfo, &name);
			break;
		case 2: /* Windows 2003 */
			/* XXX complete detection using http://msdn.microsoft.com/en-us/library/ms724833%28VS.85%29.aspx */
			if( (osinfo.wProductType == VER_NT_WORKSTATION) &&
			    (sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) ) {
				name = strdup(WINDOWSXP);
				r = home_or_pro(osinfo, &name);
				r = sg_concat_string(&name, " x64 Edition");
			}
			else {
				name = strdup(WINDOWS2003);
			}

			if(name == NULL) {
				goto out;
			}
			if (osinfo.wSuiteMask & VER_SUITE_DATACENTER) {
				r = sg_concat_string(&name, " Datacenter Edition");
			} else if (osinfo.wSuiteMask & VER_SUITE_ENTERPRISE) {
				r = sg_concat_string(&name, " Enterprise Edition");
			} else if (osinfo.wSuiteMask & VER_SUITE_BLADE) {
				r = sg_concat_string(&name, " Web Edition");
			} else {
				r = sg_concat_string(&name, " Standard Edition");
			}
			break;
		default:
			name = strdup("Windows 2000 based");
			break;
	}
	if(r != 0) {
		free (name);
		return NULL;
	}
	/* Add on service pack version */
	if (osinfo.wServicePackMajor != 0) {
		if(osinfo.wServicePackMinor == 0) {
			if(snprintf(tmp, sizeof(tmp), " SP%d", osinfo.wServicePackMajor) != -1) {
				r = sg_concat_string(&name, tmp);
			}
		} else {
			if(snprintf(tmp, sizeof(tmp), " SP%d.%d", osinfo.wServicePackMajor,
					osinfo.wServicePackMinor) != -1) {
				r = sg_concat_string(&name, tmp);
			}
		}
		if(r) {
			free(name);
			return NULL;
		}
	}
	return name;

out:
	/* strdup failed */
	sg_set_error_with_errno(SG_ERROR_MALLOC, NULL);
	ERROR_LOG("os", "get_os_name: strdup() failed");
	return NULL;
}
#endif

#ifdef GET_BITWIDTH_BY_ARCH_NAME
static int
cmp_arch_name(const void *va, const void *vb) {
	const char *a = * (char * const *)va;
	const char *b = * (char * const *)vb;
	size_t len, len_b;

	assert(a);
	assert(b);

	len = strlen(a);
	len_b = strlen(b);
	if( len_b < len )
		len = len_b;

	return strncmp(a, b, len);
}

static unsigned
get_bitwidth_by_arch_name(const char *arch_name) {
	char *m = bsearch( &arch_name, &isa64, lengthof(isa64), sizeof(isa64[0]), cmp_arch_name);
	if( m != NULL )
		return 64;
	m = bsearch( &arch_name, &isa32, lengthof(isa32), sizeof(isa32[0]), cmp_arch_name);
	if( m != NULL )
		return 32;
	return 0;
}
#endif

static sg_error
sg_get_host_info_int(sg_host_info *host_info_buf) {

#ifdef WIN32
	unsigned long nameln;
	char *name;
	long long result;
	OSVERSIONINFOEX osinfo;
	SYSTEM_INFO sysinfo;
	char *tmp_name;
	char tmp[10];
#else
	struct utsname os;
# if defined(HPUX)
	struct pst_static pstat_static;
	struct pst_dynamic pstat_dynamic;
	time_t currtime;
	long boottime;
# elif defined(SOLARIS)
	time_t boottime, curtime;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;
	char *isainfo = NULL;
	long isabufsz, rc;
# elif defined(LINUX) || defined(CYGWIN)
	FILE *f;
# elif defined(ALLBSD)
	int mib[2];
	struct timeval boottime;
	time_t curtime;
	size_t size;
	int ncpus;
#  if defined(HW_MACHINE_ARCH)
	char arch_name[16];
#  endif
# elif defined(AIX)
	static perfstat_cpu_total_t cpu_total;
	sg_error rc;
#  if defined(HAVE_GETUTXENT)
	struct utmpx *ut;
#  else
	struct utmp *ut;
#  endif
# endif
#endif

	host_info_buf->ncpus = 0;
	host_info_buf->maxcpus = 0;
	host_info_buf->bitwidth = 0;
	host_info_buf->host_state = sg_unknown_configuration;
	host_info_buf->uptime = 0;
	host_info_buf->systime = 0;

#ifdef WIN32
	/* these settings are static after boot, so why get them
	 * constantly?
	 *
	 * Because we want to know some changes anyway - at least
	 * when the hostname (DNS?) changes
	 */

	/* get system name */
	nameln = MAX_COMPUTERNAME_LENGTH + 1;
	name = sg_malloc(nameln);
	if(name == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	/*
	 * XXX probably GetComputerNameEx() is a better entry point ...
	 */
	if( GetComputerName(name, &nameln) == 0 ) {
		free(name);
		RETURN_WITH_SET_ERROR("os", SG_ERROR_HOST, "GetComputerName");
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->hostname, name)) {
		free(name);
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}
	free(name);

	/* get OS name, version and build */
	ZeroMemory(&osinfo, sizeof(OSVERSIONINFOEX));
	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	if(!GetVersionEx(&osinfo)) {
		RETURN_WITH_SET_ERROR("os", SG_ERROR_HOST, "GetVersionEx");
	}

	/* Release - single number */
	if(snprintf(tmp, sizeof(tmp), "%ld", osinfo.dwBuildNumber) == -1) {
		free(tmp);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SPRINTF, NULL);
	}
	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_release, tmp)) {
		free(tmp);
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	/* Version */
	/* usually a single digit . single digit, eg 5.0 */
	if(snprintf(tmp, sizeof(tmp), "%ld.%ld", osinfo.dwMajorVersion,
				osinfo.dwMinorVersion) == -1) {
		free(tmp);
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}
	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_version, tmp)) {
		free(tmp);
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	/* OS name */
	tmp_name = get_os_name(osinfo);
	if(tmp_name == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_name, tmp_name)) {
		free(tmp_name);
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}
	free(tmp_name);

	/* Platform */
	GetSystemInfo(&sysinfo);
	switch(sysinfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_INTEL:
			if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform,
						"Intel")) {
				RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
			}
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform,
						"IA64")) {
				RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
			}
			break;
		case PROCESSOR_ARCHITECTURE_AMD64:
			if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform,
						"AMD64")) {
				RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
			}
			break;
		default:
			if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform,
						"Unknown")){
				RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
			}
			break;
	}

	if(read_counter_large(SG_WIN32_UPTIME, &result)) {
		RETURN_WITH_SET_ERROR("os", SG_ERROR_PDHREAD, PDH_UPTIME);
	}

	host_info_buf->uptime = (time_t) result;
#else
	if((uname(&os)) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_UNAME, NULL);
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_name, os.sysname)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_release, os.release)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->os_version, os.version)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform, os.machine)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->hostname, os.nodename)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	/* get uptime */
#ifdef HPUX
	if (pstat_getstatic(&pstat_static, sizeof(pstat_static), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_PSTAT, "pstat_static");
	}

	if (pstat_getdynamic(&pstat_dynamic, sizeof(pstat_dynamic), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_PSTAT, "pstat_dynamic");
	}

	currtime = time(NULL);

	boottime = pstat_static.boot_time;

	host_info_buf->uptime = currtime - boottime;
	host_info_buf->ncpus = pstat_dynamic.psd_proc_cnt;
	host_info_buf->maxcpus = pstat_dynamic.psd_max_proc_cnt;
	host_info_buf->bitwidth = sysconf(_SC_KERNEL_BITS);

	/*
	 * TODO: getting virtualization state
	 *       1) on boostrapping this component, try loading /opt/hpvm/lib/libhpvm.so (or so)
	 *       2) get function addresses for
	 *          a) HPVM_boolean hpvm_api_server_check()
	 *          b) HPVM_boolean hpvm_api_virtmach_check()
	 *
	 * Seems to be hardware virtualization ...
	 * See: http://docstore.mik.ua/manuals/hp-ux/en/T2767-90141/index.html (hpvmpubapi(3))
	 *      http://jreypo.wordpress.com/tag/hpvm/
	 *      http://jreypo.wordpress.com/category/hp-ux/page/3/
	 *      http://h20338.www2.hp.com/enterprise/us/en/os/hpux11i-partitioning-integrity-vm.html
	 */

#elif defined(SOLARIS)
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("os", SG_ERROR_KSTAT_OPEN, NULL);
	}

	if((ksp=kstat_lookup(kc, "unix", -1, "system_misc"))==NULL){
		kstat_close(kc);
		RETURN_WITH_SET_ERROR("os", SG_ERROR_KSTAT_LOOKUP, "unix,-1,system_misc");
	}
	if (kstat_read(kc, ksp, 0) == -1) {
		kstat_close(kc);
		RETURN_WITH_SET_ERROR("os", SG_ERROR_KSTAT_READ, NULL);
	}
	if((kn=kstat_data_lookup(ksp, "boot_time")) == NULL){
		kstat_close(kc);
		RETURN_WITH_SET_ERROR("os", SG_ERROR_KSTAT_DATA_LOOKUP, "boot_time");
	}
	/* XXX verify on Solaris 10 if it's still ui32 */
	boottime = (kn->value.ui32);

	kstat_close(kc);

	time(&curtime);
	host_info_buf->uptime = curtime - boottime;

	host_info_buf->ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	host_info_buf->maxcpus = sysconf(_SC_NPROCESSORS_CONF);
	isainfo = sg_malloc( isabufsz = (32 * sizeof(*isainfo)) );
	if( NULL == isainfo ) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}
# define MKSTR(x) #x
# if defined(SI_ARCHITECTURE_K)
#  define SYSINFO_CMD SI_ARCHITECTURE_K
# elif defined(SI_ISALIST)
#  define SYSINFO_CMD SI_ISALIST
# else
#  define SYSINFO_CMD SI_ARCHITECTURE
# endif
sysinfo_again:
	if( -1 == ( rc = sysinfo( SYSINFO_CMD, isainfo, isabufsz ) ) ) {
		free(isainfo);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSINFO, MKSTR(SYSINFO_CMD) );
	}
	else if( rc > isabufsz ) {
		char *tmp = sg_realloc(isainfo, rc);
		if( NULL == tmp ) {
			free(isainfo);
			RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
		}
		isabufsz = rc;
		isainfo = tmp;
		goto sysinfo_again;
	}

	host_info_buf->bitwidth = get_bitwidth_by_arch_name(isainfo);
	free(isainfo);
	host_info_buf->host_state = sg_unknown_configuration;

#elif defined(LINUX) || defined(CYGWIN)
	if ((f=fopen("/proc/uptime", "r")) == NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_OPEN, "/proc/uptime");
	}

#define TIME_T_SCANF_FMT (sizeof(int[(((time_t)-1)/2)%4+1]) == sizeof(int[1]) ? "%ld %*d" : "%lu %*d" )
	if((fscanf(f,TIME_T_SCANF_FMT,&host_info_buf->uptime)) != 1){
		fclose(f);
		RETURN_WITH_SET_ERROR("os", SG_ERROR_PARSE, NULL);
	}
	fclose(f);

# if defined(LINUX)
	host_info_buf->ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	host_info_buf->maxcpus = sysconf(_SC_NPROCESSORS_CONF);
	if( access( "/proc/sys/kernel/vsyscall64", F_OK ) == 0 ||
	    access( "/proc/sys/abi/vsyscall32", F_OK ) == 0 ) {
		host_info_buf->bitwidth = 64;
	}
	else {
		host_info_buf->bitwidth = sysconf(_SC_LONG_BIT); // well, maybe 64-bit disabled 128-bit system o.O
	}
	host_info_buf->host_state = sg_unknown_configuration;
# endif

#elif defined(ALLBSD)
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSCTL, "CTL_KERN.KERN_BOOTTIME");
	}
	time(&curtime);
	host_info_buf->uptime= curtime - boottime.tv_sec;

# if defined(HW_NCPU)
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	size = sizeof(int);
	if( sysctl( mib, 2, &ncpus, &size, NULL, 0 ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSCTL, "CTL_HW.HW_NCPU" );
	}
# endif
# if defined(HW_MACHINE_ARCH)
	mib[0] = CTL_HW;
	mib[1] = HW_MACHINE_ARCH;
	size = sizeof(arch_name);
	if( sysctl( mib, 2, arch_name, &size, NULL, 0 ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSCTL, "CTL_HW.HW_MACHINE_ARCH" );
	}
	host_info_buf->bitwidth = get_bitwidth_by_arch_name(arch_name);
# endif
	host_info_buf->host_state = sg_unknown_configuration; /* details must be analysed "manually", no syscall */
	host_info_buf->maxcpus = ncpus;
# if defined(HW_NCPUONLINE)
	/* use knowledge about number of cpu's online, when available instead of assuming all of them */
	mib[0] = CTL_HW;
	mib[1] = HW_NCPUONLINE;
	size = sizeof(int);
	if( sysctl( mib, 2, &ncpus, &size, NULL, 0 ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSCTL, "CTL_HW.HW_NCPUONLINE" );
	}
# endif
	host_info_buf->ncpus = ncpus;

#elif defined(AIX)
	if(perfstat_cpu_total(NULL, &cpu_total, sizeof(cpu_total), 1) != 1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("os", SG_ERROR_SYSCTL, "perfstat_cpu_total");
	}

	if(SG_ERROR_NONE != sg_update_string(&host_info_buf->platform, cpu_total.description)) {
		RETURN_FROM_PREVIOUS_ERROR( "os", sg_get_error() );
	}

	host_info_buf->ncpus = cpu_total.ncpus;
	host_info_buf->maxcpus = cpu_total.ncpus_cfg;
	host_info_buf->bitwidth = sysconf(_SC_AIX_KERNEL_BITMODE);
	if( sysconf(_SC_LPAR_ENABLED) > 0 ) {
		host_info_buf->host_state = sg_hardware_virtualized;
	}
	else {
		host_info_buf->host_state = sg_physical_host;
	}

	if( SG_ERROR_NONE != ( rc = sg_lock_mutex("utmp") ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "os", rc );
	}
# if defined(HAVE_GETUTXENT)
#  define UTENTFN getutxent
#  define UTENTTM ut->ut_tv.tv_sec
	setutxent();
# else
#  define UTENTFN getutent
#  define UTENTTM ut->ut_time
	setutent();
# endif
	while( NULL != ( ut = UTENTFN() ) ) {
		if( ut->ut_type == BOOT_TIME ) {
			host_info_buf->uptime = time(NULL) - UTENTTM;
			break;
		}
	}
# if defined(HAVE_GETUTXENT)
	endutxent();
# else
	endutent();
# endif
	if( SG_ERROR_NONE != ( rc = sg_unlock_mutex("utmp") ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "os", rc );
	}
#else
	RETURN_WITH_SET_ERROR("os", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif
#endif /* WIN32 */

	host_info_buf->systime = time(NULL);

	return SG_ERROR_NONE;
}

#ifdef AIX
EASY_COMP_SETUP(os,1,"utmp",NULL);
#else
EASY_COMP_SETUP(os,1,NULL);
#endif

typedef sg_host_info sg_os_stats;

static void
sg_os_stats_item_init(sg_os_stats *d) {

	memset( d, 0, sizeof(*d) );
}

#if 0
#else
#define sg_os_stats_item_copy NULL
#define sg_os_stats_item_compute_diff NULL
#define sg_os_stats_item_compare NULL
#endif

static void
sg_os_stats_item_destroy(sg_os_stats *d) {

	free(d->os_name);
	free(d->os_release);
	free(d->os_version);
	free(d->platform);
	free(d->hostname);
	memset( d, 0, sizeof(*d) );
}

VECTOR_INIT_INFO_FULL_INIT(sg_os_stats);

#define SG_OS_CUR_IDX 0

EASY_COMP_ACCESS(sg_get_host_info,os,os,SG_OS_CUR_IDX)
