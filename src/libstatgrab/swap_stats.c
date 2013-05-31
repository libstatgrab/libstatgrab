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

#define __NEED_SG_GET_SYS_PAGE_SIZE
#include "tools.h"

#if defined(HAVE_STRUCT_XSWDEV)
EXTENDED_COMP_SETUP(swap,1,NULL);

static int swapinfo_mib[2];
static bool swapinfo_array = false;
static char *swapinfo_sysctl_name;

sg_error
sg_swap_init_comp(unsigned id) {
	size_t len = lengthof(swapinfo_mib);
	GLOBAL_SET_ID(swap,id);

	if( sysctlnametomib("vm.swap_info", swapinfo_mib, &len) < 0 ) {
		if( sysctlnametomib("vm.swap_info_array", swapinfo_mib, &len) < 0 ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLNAMETOMIB, "vm.swap_info + vm.swap_info_array");
		}
		else {
			swapinfo_array = true;
			swapinfo_sysctl_name = "vm.swap_info_array";
		}
	}
	else {
		swapinfo_array = false;
		swapinfo_sysctl_name = "vm.swap_info";
	}

	return SG_ERROR_NONE;
}

EASY_COMP_DESTROY_FN(swap)
EASY_COMP_CLEANUP_FN(swap,1)
#else
EASY_COMP_SETUP(swap,1,NULL);
#endif

static sg_error
sg_get_swap_stats_int(sg_swap_stats *swap_stats_buf) {

#ifdef HPUX
#define SWAP_BATCH 5
	struct pst_swapinfo pstat_swapinfo[SWAP_BATCH];
	int swapidx = 0;
	int num, i;
#elif defined(SOLARIS)
# if defined(HAVE_STRUCT_SWAPTABLE)
	struct swaptable *swtbl;
	int nswap, i;
# elif defined(HAVE_STRUCT_ANONINFO)
	struct anoninfo ai;
# endif
	int pagesize;
#elif defined(LINUX) || defined(CYGWIN)
	FILE *f;
#define LINE_BUF_SIZE 256
	char line_buf[LINE_BUF_SIZE];
	unsigned long long value;
	unsigned matches = 0;
#elif defined(HAVE_STRUCT_XSWDEV) || defined(HAVE_STRUCT_XSW_USAGE)
	int pagesize;
# if defined(HAVE_STRUCT_XSWDEV)
	struct xswdev xsw;
	struct xswdev *xswbuf = NULL, *xswptr = NULL;
# elif defined(HAVE_STRUCT_XSW_USAGE)
	struct xsw_usage xsw;
# endif
	int mib[16], n;
	size_t mibsize, size;
#elif defined(HAVE_STRUCT_UVMEXP_SYSCTL) && defined(VM_UVMEXP2)
	int mib[2] = { CTL_VM, VM_UVMEXP2 };
	struct uvmexp_sysctl uvm;
	size_t size = sizeof(uvm);
#elif defined(HAVE_STRUCT_UVMEXP) && defined(VM_UVMEXP)
	int mib[2] = { CTL_VM, VM_UVMEXP };
	struct uvmexp uvm;
	size_t size = sizeof(uvm);
#elif defined(ALLBSD)
	/* fallback if no reasonable API is supported */
	int pagesize;
	struct kvm_swap swapinfo;
	kvm_t *kvmd;
#elif defined(AIX)
	perfstat_memory_total_t mem;
	long long pagesize;
#elif defined(WIN32)
	MEMORYSTATUSEX memstats;
#endif

	swap_stats_buf->total = 0;
	swap_stats_buf->used = 0;
	swap_stats_buf->free = 0;

#ifdef HPUX
	for(;;) {
		num = pstat_getswap(pstat_swapinfo, sizeof pstat_swapinfo[0],
				    SWAP_BATCH, swapidx);
		if (num == -1) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_PSTAT, "pstat_getswap");
		}
		else if (num == 0) {
			break;
		}

		for (i = 0; i < num; ++i) {
			struct pst_swapinfo *si = &pstat_swapinfo[i];

			if ((si->pss_flags & SW_ENABLED) != SW_ENABLED)
				continue;

			if ((si->pss_flags & SW_BLOCK) == SW_BLOCK) {
				swap_stats_buf->total += ((long long) si->pss_nblksavail) * 1024LL;
				swap_stats_buf->used += ((long long) si->pss_nfpgs) * 1024LL;
				swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
			}

			if ((si->pss_flags & SW_FS) == SW_FS) {
				swap_stats_buf->total += ((long long) si->pss_limit) * 1024LL;
				swap_stats_buf->used += ((long long) si->pss_allocated) * 1024LL;
				swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
			}
		}
		swapidx = pstat_swapinfo[num - 1].pss_idx + 1;
	}
#elif defined(AIX)
	if ((pagesize = sysconf(_SC_PAGESIZE)) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

	/* return code is number of structures returned */
	if(perfstat_memory_total(NULL, &mem, sizeof(perfstat_memory_total_t), 1) != 1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTLBYNAME, "perfstat_memory_total");
	}

	swap_stats_buf->total = ((long long)mem.pgsp_total) * pagesize;
	swap_stats_buf->free  = ((long long)mem.pgsp_free)  * pagesize;
	swap_stats_buf->used  = swap_stats_buf->total - swap_stats_buf->free;
#elif defined(SOLARIS)
	if( -1 == ( pagesize = sg_get_sys_page_size() ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "swap", sg_get_error() );
	}

# if defined(HAVE_STRUCT_SWAPTABLE)
again:
	if( ( nswap = swapctl(SC_GETNSWP, 0) ) == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SWAPCTL, NULL);
	}
	if( nswap != 0 ) {
		char *buf = sg_malloc( nswap * sizeof(char) * (PATH_MAX+1) + 1 );
		if( NULL == buf ) {
			RETURN_FROM_PREVIOUS_ERROR( "swap", sg_get_error() );
		}

		swtbl = sg_malloc( sizeof(*swtbl) + ( nswap * sizeof(swtbl->swt_ent[0]) ) );
		if( NULL == swtbl ) {
			free(buf);
			RETURN_FROM_PREVIOUS_ERROR( "swap", sg_get_error() );
		}

		memset( buf, 0, nswap * sizeof(char) * (PATH_MAX+1) + 1 );
		memset( swtbl, 0, sizeof(*swtbl) + ( nswap * sizeof(swtbl->swt_ent[0]) ) );

		for( i = 0; i < nswap; ++i )
			swtbl->swt_ent[i].ste_path = buf + (i * (PATH_MAX+1));

		swtbl->swt_n = nswap;
		if ((i = swapctl(SC_LIST, swtbl)) < 0) {
			free( buf );
			free( swtbl );
			RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SWAPCTL, NULL);
		}
		if( i > nswap ) {
			free( swtbl );
			free( buf );
			goto again;
		}
		for( i = 0; i < nswap; ++i ) {
			swap_stats_buf->total = swtbl->swt_ent[i].ste_pages;
			swap_stats_buf->free = swtbl->swt_ent[i].ste_free;
		}
		free( swtbl );
		free( buf );
		swap_stats_buf->total *= pagesize;
		swap_stats_buf->free *= pagesize;
		swap_stats_buf->used = swap_stats_buf->total - swap_stats_buf->free;
	}
# elif defined(HAVE_STRUCT_ANONINFO)
	if (swapctl(SC_AINFO, &ai) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SWAPCTL, NULL);
	}

	swap_stats_buf->total = ai.ani_max;
	swap_stats_buf->total *= pagesize;
	swap_stats_buf->used = ai.ani_resv;
	swap_stats_buf->used *= pagesize;
	swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
# else
	RETURN_WITH_SET_ERROR("swap", SG_ERROR_UNSUPPORTED, OS_TYPE);
# endif
#elif defined(LINUX) || defined(CYGWIN)
	if ((f = fopen("/proc/meminfo", "r")) == NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_OPEN, "/proc/meminfo");
	}

	while( (matches < 2) && (fgets(line_buf, sizeof(line_buf), f) != NULL) ) {
		if (sscanf(line_buf, "%*s %llu kB", &value) != 1)
			continue;
		value *= 1024;

		if (strncmp(line_buf, "SwapTotal:", 10) == 0) {
			swap_stats_buf->total = value;
			++matches;
		}
		else if (strncmp(line_buf, "SwapFree:", 9) == 0) {
			swap_stats_buf->free = value;
			++matches;
		}
	}

	fclose(f);

	if( matches < 2 ) {
		RETURN_WITH_SET_ERROR( "swap", SG_ERROR_PARSE, "/proc/meminfo" );
	}

	swap_stats_buf->used = swap_stats_buf->total - swap_stats_buf->free;
#elif defined(HAVE_STRUCT_XSWDEV) || defined(HAVE_STRUCT_XSW_USAGE)
	pagesize=getpagesize();

# if defined(HAVE_STRUCT_XSWDEV)
	mibsize = 2;
	if( swapinfo_array ) {
		size = 0;
		if( sysctl( swapinfo_mib, 2, NULL, &size, NULL, 0 ) < 0 ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, swapinfo_sysctl_name);
		}
		if( NULL == ( xswbuf = sg_malloc( size ) ) ) {
			RETURN_FROM_PREVIOUS_ERROR( "swap", sg_get_error() );
		}
		if( sysctl( swapinfo_mib, 2, xswbuf, &size, NULL, 0 ) < 0 ) {
			RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, swapinfo_sysctl_name);
		}
	}
	else {
		mib[0] = swapinfo_mib[0];
		mib[1] = swapinfo_mib[1];
	}
# elif defined(HAVE_STRUCT_XSW_USAGE)
	mib[0] = CTL_VM;
	mib[1] = VM_SWAPUSAGE;
	mibsize = 2;
# endif
	for (n = 0; ; ++n) {
# if defined(HAVE_STRUCT_XSWDEV)
		if( !swapinfo_array )
# else
		if( 1 )
# endif
		{
			mib[mibsize] = n;
			size = sizeof(xsw);

			if (sysctl(mib, mibsize + 1, &xsw, &size, NULL, 0) < 0) {
				if (errno == ENOENT)
					break;
				if( xswbuf )
					free( xswbuf );
# if defined(HAVE_STRUCT_XSWDEV)
				RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, swapinfo_sysctl_name);
# else
				RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, "CTL_VM.VM_SWAPUSAGE" );
# endif
			}

			xswptr = &xsw;
		}
# if defined(HAVE_STRUCT_XSWDEV) && defined(HAVE_STRUCT_XSWDEV_SIZE)
		else {
			if( n >= (size / xswbuf->xsw_size) )
				break;
			xswptr = xswbuf + n;
		}
# endif

		if( xswptr == NULL ) {
			RETURN_WITH_SET_ERROR("swap", SG_ERROR_MEMSTATUS, "no swap status");
		}

# ifdef XSWDEV_VERSION
		if( xswptr->xsw_version != XSWDEV_VERSION ) {
			if( xswbuf )
				free( xswbuf );
			RETURN_WITH_SET_ERROR("swap", SG_ERROR_XSW_VER_MISMATCH, NULL);
		}
# endif

# if defined(HAVE_STRUCT_XSWDEV)
		swap_stats_buf->total += (unsigned long long) xswptr->xsw_nblks;
		swap_stats_buf->used += (unsigned long long) xswptr->xsw_used;
# elif defined(HAVE_STRUCT_XSW_USAGE)
		swap_stats_buf->total += (unsigned long long) xswptr->xsu_total;
		swap_stats_buf->used += (unsigned long long) xswptr->xsu_used;
		swap_stats_buf->free += (unsigned long long) xswptr->xsu_avail;
# endif
	}

	if( xswbuf )
		free( xswbuf );

	swap_stats_buf->total *= pagesize;
	swap_stats_buf->used *= pagesize;
	if( 0 == swap_stats_buf->free )
		swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
	else
		swap_stats_buf->free *= pagesize;
#elif defined(HAVE_STRUCT_UVMEXP_SYSCTL) && defined(VM_UVMEXP2)
	if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP2");
	}

	swap_stats_buf->total = uvm.pagesize * uvm.swpages;
	swap_stats_buf->used = uvm.pagesize * uvm.swpginuse; /* XXX swpgonly ? */
	swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
#elif defined(HAVE_STRUCT_UVMEXP) && defined(VM_UVMEXP)
	if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP");
	}

	swap_stats_buf->total = (unsigned long long)uvm.pagesize * (unsigned long long)uvm.swpages;
	swap_stats_buf->used = (unsigned long long)uvm.pagesize * (unsigned long long)uvm.swpginuse; /* XXX swpgonly ? */
	swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
#elif defined(ALLBSD)
	/* XXX probably not mt-safe! */
	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL);
	if(kvmd == NULL) {
		RETURN_WITH_SET_ERROR("swap", SG_ERROR_KVM_OPENFILES, NULL);
	}

	if ((kvm_getswapinfo(kvmd, &swapinfo, 1,0)) == -1) {
		kvm_close( kvmd );
		RETURN_WITH_SET_ERROR("swap", SG_ERROR_KVM_GETSWAPINFO, NULL);
	}

	swap_stats_buf->total = (long long)swapinfo.ksw_total;
	swap_stats_buf->used = (long long)swapinfo.ksw_used;
	kvm_close( kvmd );

	swap_stats_buf->total *= pagesize;
	swap_stats_buf->used *= pagesize;
	swap_stats_buf->free = swap_stats_buf->total - swap_stats_buf->used;
#elif defined(WIN32)
	memstats.dwLength = sizeof(memstats);
	if (!GlobalMemoryStatusEx(&memstats)) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("swap", SG_ERROR_MEMSTATUS, "GloblaMemoryStatusEx");
	}

	/* the PageFile stats include Phys memory "minus an overhead".
	 * Due to this unknown "overhead" there's no way to extract just page
	 * file use from these numbers */
	swap_stats_buf->total = memstats.ullTotalPageFile;
	swap_stats_buf->free = memstats.ullAvailPageFile;
	swap_stats_buf->used = swap_stats_buf->total - swap_stats_buf->free;
#else
	RETURN_WITH_SET_ERROR("swap", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	swap_stats_buf->systime = time(NULL);

	return SG_ERROR_NONE;
}

VECTOR_INIT_INFO_EMPTY_INIT(sg_swap_stats);

#define SG_SWAP_CUR_IDX 0

EASY_COMP_ACCESS(sg_get_swap_stats,swap,swap,SG_SWAP_CUR_IDX)
