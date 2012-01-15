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

#if defined(HAVE_STRUCT_VMTOTAL) && \
    !(defined(HAVE_STRUCT_UVMEXP_SYSCTL) && defined(VM_UVMEXP2)) && \
    !(defined(HAVE_STRUCT_UVMEXP) && defined(VM_UVMEXP))
EXTENDED_COMP_SETUP(mem,1,NULL);

static int vmtotal_mib[2];

sg_error
sg_mem_init_comp(unsigned id) {
	size_t len = lengthof(vmtotal_mib);
	GLOBAL_SET_ID(mem,id);

	if( -1 == sysctlnametomib( "vm.vmtotal", vmtotal_mib, &len ) ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLNAMETOMIB, "vm.vmtotal");
	}

	return SG_ERROR_NONE;
}

EASY_COMP_DESTROY_FN(mem)
EASY_COMP_CLEANUP_FN(mem,1)
#else
EASY_COMP_SETUP(mem,1,NULL);
#endif

static sg_error
sg_get_mem_stats_int(sg_mem_stats *mem_stats_buf) {

#ifdef HPUX
	struct pst_static pstat_static;
	struct pst_dynamic pstat_dynamic;
#if 0
	long long pagesize;
#endif
#elif defined(SOLARIS)
# ifdef _SC_PHYS_PAGES
	long phystotal;
	long physav;
# else
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;
# endif
	long pagesize;
#elif defined(LINUX) || defined(CYGWIN)
#define LINE_BUF_SIZE 256
	char *line_ptr, line_buf[LINE_BUF_SIZE];
	long long value;
	FILE *f;
#elif defined(HAVE_STRUCT_UVMEXP_SYSCTL) && defined(VM_UVMEXP2)
	int mib[2];
	struct uvmexp_sysctl uvm;
	size_t size = sizeof(uvm);
#elif defined(HAVE_STRUCT_UVMEXP) && defined(VM_UVMEXP)
	int mib[2];
	struct uvmexp uvm;
	size_t size = sizeof(uvm);
#elif defined(HAVE_STRUCT_VMTOTAL)
	struct vmtotal vmtotal;
	size_t size;
	int pagesize, page_multiplier;
#ifdef HW_PHYSMEM
	int mib[2];
	u_long total_mem;
#endif
#elif defined(FREEBSD) || defined(DFBSD)
	int mib[2];
	u_long physmem;
	size_t size;
	u_long free_count;
	u_long cache_count;
	u_long inactive_count;
	int pagesize;
#elif defined(AIX)
	perfstat_memory_total_t mem;
	long long pagesize;
#elif defined(WIN32)
	MEMORYSTATUSEX memstats;
#endif

#if defined(HPUX)
#if 0
	if((pagesize=sysconf(_SC_PAGESIZE)) == -1){
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}
#endif
	if (pstat_getdynamic(&pstat_dynamic, sizeof(pstat_dynamic), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_PSTAT, "pstat_dynamic");
	}

	/*
	 * from man pstat_getstatic:
	 *
	 * pstat_getstatic() returns information about the system.  Although
	 * this data usually does not change frequently, it may change while
	 * the system is running due to manually or automatically generated
	 * administrative changes in the associated kernel tunables, online
	 * addition/deletion of resources, or other events.  There is one
	 * global instance of this context.
	 *
	 * ==> Can't hold this value globally static.
	 */

	if( pstat_getstatic(&pstat_static, sizeof(pstat_static), 1, 0) == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_PSTAT, "pstat_static");
        }

	mem_stats_buf->total = ((long long) pstat_static.physical_memory) * pstat_static.page_size;
	mem_stats_buf->free = ((long long) pstat_dynamic.psd_free) * pstat_static.page_size;
	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;
#elif defined(AIX)
	if((pagesize = sysconf(_SC_PAGESIZE)) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

	/* return code is number of structures returned */
	if(perfstat_memory_total(NULL, &mem, sizeof(perfstat_memory_total_t), 1) != 1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLBYNAME, "perfstat_memory_total");
	}

	mem_stats_buf->total = ((unsigned long long) mem.real_total) * pagesize;
	mem_stats_buf->free  = ((unsigned long long) mem.real_free)  * pagesize;
	mem_stats_buf->used  = ((unsigned long long) mem.real_inuse) * pagesize;
	mem_stats_buf->cache = ((unsigned long long) mem.numperm)    * pagesize;
#elif defined(SOLARIS)
	if( ( pagesize=sysconf(_SC_PAGESIZE) ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

#ifdef _SC_PHYS_PAGES
	if( ( phystotal = sysconf(_SC_PHYS_PAGES) ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCONF, "_SC_PHYS_PAGES");
	}
	if( ( physav = sysconf(_SC_AVPHYS_PAGES) ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCONF, "_SC_AVPHYS_PAGES");
	}
	mem_stats_buf->total = ((unsigned long long)phystotal) * ((unsigned long long)pagesize);
	mem_stats_buf->free = ((unsigned long long)physav) * ((unsigned long long)pagesize);
	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;
	mem_stats_buf->cache = 0;
#else
	if( (kc = kstat_open()) == NULL ) {
		RETURN_WITH_SET_ERROR("mem", SG_ERROR_KSTAT_OPEN, NULL);
        }

	if((ksp=kstat_lookup(kc, "unix", 0, "system_pages")) == NULL) {
		RETURN_WITH_SET_ERROR("mem", SG_ERROR_KSTAT_LOOKUP, "unix,0,system_pages");
        }

	if (kstat_read(kc, ksp, 0) == -1) {
		RETURN_WITH_SET_ERROR("mem", SG_ERROR_KSTAT_READ, NULL);
        }

	if((kn=kstat_data_lookup(ksp, "physmem")) == NULL) {
		RETURN_WITH_SET_ERROR("mem", SG_ERROR_KSTAT_DATA_LOOKUP, "physmem");
        }

	mem_stats_buf->total = ((unsigned long long)kn->value.ul) * ((unsigned long long)pagesize);

	if((kn=kstat_data_lookup(ksp, "freemem")) == NULL) {
		RETURN_WITH_SET_ERROR("mem", SG_ERROR_KSTAT_DATA_LOOKUP, "freemem");
        }

	mem_stats_buf->free = ((unsigned long long)kn->value.ul) * ((unsigned long long)pagesize);
	kstat_close(kc);

	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;
	mem_stats_buf->cache = 0;
#endif
#elif defined(LINUX) || defined(CYGWIN)
	if ((f = fopen("/proc/meminfo", "r")) == NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_OPEN, "/proc/meminfo");
	}

#define MEM_TOTAL_PREFIX	"MemTotal:"
#define MEM_FREE_PREFIX		"MemFree:"
#define MEM_CACHED_PREFIX	"Cached:"

	while ((line_ptr = fgets(line_buf, sizeof(line_buf), f)) != NULL) {
		if ( sscanf(line_buf, "%*s %lld kB", &value) != 1)
			continue;

		if (strncmp(line_buf, MEM_TOTAL_PREFIX, sizeof(MEM_TOTAL_PREFIX) - 1) == 0)
			mem_stats_buf->total = value;
		else if (strncmp(line_buf, MEM_FREE_PREFIX, sizeof(MEM_FREE_PREFIX) - 1) == 0)
			mem_stats_buf->free = value;
		else if (strncmp(line_buf, MEM_CACHED_PREFIX, sizeof(MEM_CACHED_PREFIX) - 1) == 0)
			mem_stats_buf->cache = value;
	}

	mem_stats_buf->total *= 1024;
	mem_stats_buf->free *= 1024;
	mem_stats_buf->cache *= 1024;

#undef MEM_TOTAL_PREFIX
#undef MEM_FREE_PREFIX
#undef MEM_CACHED_PREFIX

	fclose(f);
	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;

#elif defined(HAVE_STRUCT_UVMEXP_SYSCTL) && defined(VM_UVMEXP2)
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP2;

	if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP2");
	}

	mem_stats_buf->total = uvm.npages;
	mem_stats_buf->cache = uvm.filepages + uvm.execpages;
	mem_stats_buf->free = uvm.free;

	mem_stats_buf->total *= uvm.pagesize;
	mem_stats_buf->cache *= uvm.pagesize;
	mem_stats_buf->free *= uvm.pagesize;

	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;
#elif defined(HAVE_STRUCT_UVMEXP) && defined(VM_UVMEXP)
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;

	if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP");
	}

	mem_stats_buf->total = uvm.npages;
	mem_stats_buf->cache = 0;
# if defined(HAVE_STRUCT_UVMEXP_FILEPAGES)
	mem_stats_buf->cache += uvm.filepages;
# endif
# if defined(HAVE_STRUCT_UVMEXP_EXECPAGES)
	mem_stats_buf->cache += uvm.execpages;
# endif
	mem_stats_buf->free = uvm.free;

	mem_stats_buf->total *= uvm.pagesize;
	mem_stats_buf->cache *= uvm.pagesize;
	mem_stats_buf->free *= uvm.pagesize;

	mem_stats_buf->used = mem_stats_buf->total - mem_stats_buf->free;
#elif defined(HAVE_STRUCT_VMTOTAL)
	/* The code in this section is based on the code in the OpenBSD
	 * top utility, located at src/usr.bin/top/machine.c in the
	 * OpenBSD source tree.
	 *
	 * For fun, and like OpenBSD top, we will do the multiplication
	 * converting the memory stats in pages to bytes in base 2.
	 */

	/* All memory stats in OpenBSD are returned as the number of pages.
	 * To convert this into the number of bytes we need to know the
	 * page size on this system.
	 */
	pagesize = sysconf(_SC_PAGESIZE);

	/* The pagesize gives us the base 10 multiplier, so we need to work
	 * out what the base 2 multiplier is. This means dividing
	 * pagesize by 2 until we reach unity, and counting the number of
	 * divisions required.
	 */
	page_multiplier = 0;

	while (pagesize > 1) {
		++page_multiplier;
		pagesize >>= 1;
	}

	/* We can now ret the the raw VM stats (in pages) using the
	 * sysctl interface.
	 */
	size = sizeof(vmtotal);

	if (sysctl(vmtotal_mib, 2, &vmtotal, &size, NULL, 0) < 0) {
		bzero(&vmtotal, sizeof(vmtotal));
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLBYNAME, "vm.vmtotal");
	}

	/* Convert the raw stats to bytes, and return these to the caller
	 */
	mem_stats_buf->used = vmtotal.t_rm;   /* total real mem in use */
	mem_stats_buf->used <<= page_multiplier;
	/* XXX scan top source to look how it determines cache size */
	mem_stats_buf->cache = 0;				  /* no cache stats */
	mem_stats_buf->free = vmtotal.t_free; /* free memory pages */
	mem_stats_buf->free <<= page_multiplier;
# ifdef HW_PHYSMEM
	mib[0] = CTL_HW;
	mib[1] = HW_PHYSMEM;
	size = sizeof(total_mem);
	if (sysctl(mib, 2, &total_mem, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTL, "CTL_HW.HW_PHYSMEM");
	}
	mem_stats_buf->total = total_mem;
# else
	mem_stats_buf->total = (mem_stats_buf->used + mem_stats_buf->free);
# endif
#elif defined(FREEBSD) || defined(DFBSD)
	/* Returns bytes */
	mib[0] = CTL_HW;
	mib[1] = HW_PHYSMEM;
	size = sizeof(physmem);
	if (sysctl(mib, 2, &physmem, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTL, "CTL_HW.HW_PHYSMEM");
	}
	mem_stats_buf->total = physmem;

	/*returns pages*/
	size = sizeof(free_count);
	if (sysctlbyname("vm.stats.vm.v_free_count", &free_count, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_free_count");
	}

	size = sizeof(inactive_count);
	if (sysctlbyname("vm.stats.vm.v_inactive_count", &inactive_count , &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_inactive_count");
	}

	size = sizeof(cache_count);
	if (sysctlbyname("vm.stats.vm.v_cache_count", &cache_count, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_cache_count");
	}

	/* Because all the vm.stats returns pages, I need to get the page size.
	 * After that I then need to multiple the anything that used vm.stats to
	 * get the system statistics by pagesize 
	 */
	pagesize = getpagesize();
	mem_stats_buf->cache = cache_count * pagesize;

	/* Of couse nothing is ever that simple :) And I have inactive pages to
	 * deal with too. So I'm going to add them to free memory :)
	 */
	mem_stats_buf->free = (free_count * pagesize) + (inactive_count * pagesize);
	mem_stats_buf->used = physmem - mem_stats_buf->free;
#elif defined(WIN32)
	memstats.dwLength = sizeof(memstats);
	if (!GlobalMemoryStatusEx(&memstats)) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("mem", SG_ERROR_MEMSTATUS, NULL);
	}

	mem_stats_buf->free = memstats.ullAvailPhys;
	mem_stats_buf->total = memstats.ullTotalPhys;
	mem_stats_buf->used = mem_stat.total - mem_stat.free;
	if(read_counter_large(SG_WIN32_MEM_CACHE, &mem_stats_buf->cache))
		mem_stats_buf->cache = 0;
#else
	RETURN_WITH_SET_ERROR("mem", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	mem_stats_buf->systime = time(NULL);

	return SG_ERROR_NONE;
}

VECTOR_INIT_INFO_EMPTY_INIT(sg_mem_stats);

#define SG_MEM_CUR_IDX 0

EASY_COMP_ACCESS(sg_get_mem_stats,mem,mem,SG_MEM_CUR_IDX)
