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

static sg_error
sg_get_page_stats_int(sg_page_stats *page_stats_buf){
#ifdef SOLARIS
	kstat_ctl_t *kc;
	kstat_t *ksp;
	cpu_stat_t cs;
#elif defined(LINUX) || defined(CYGWIN)
	FILE *f;
#define LINE_BUF_SIZE 256
	char line_buf[LINE_BUF_SIZE];
#elif defined(FREEBSD) || defined(DFBSD)
	size_t size;
#elif defined(NETBSD) || defined(OPENBSD)
	int mib[2];
	struct uvmexp uvm;
	size_t size = sizeof(uvm);
#elif defined(AIX)
	perfstat_memory_total_t mem;
#elif defined(HPUX)
	struct pst_vminfo vminfo;
#endif

	page_stats_buf->systime = time(NULL);
	page_stats_buf->pages_pagein=0;
	page_stats_buf->pages_pageout=0;

#ifdef SOLARIS
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("page", SG_ERROR_KSTAT_OPEN, NULL);
	}

	for (ksp = kc->kc_chain; ksp!=NULL; ksp = ksp->ks_next) {
		if ((strcmp(ksp->ks_module, "cpu_stat")) != 0)
			continue;
		if (kstat_read(kc, ksp, &cs) == -1)
			continue;

		page_stats_buf->pages_pagein  += (long long)cs.cpu_vminfo.pgpgin;
		page_stats_buf->pages_pageout += (long long)cs.cpu_vminfo.pgpgout;
	}

	kstat_close(kc);
#elif defined(LINUX) || defined(CYGWIN)
	if ((f = fopen("/proc/vmstat", "r")) != NULL) {
		unsigned matches = 0;

		while( (matches < 2) && (fgets(line_buf, sizeof(line_buf), f) != NULL) ) {
			unsigned long long value;

			if (sscanf(line_buf, "%*s %llu", &value) != 1)
				continue;

			if (strncmp(line_buf, "pgpgin ", 7) == 0) {
				page_stats_buf->pages_pagein = value;
				++matches;
			}
			else if (strncmp(line_buf, "pgpgout ", 8) == 0) {
				page_stats_buf->pages_pageout = value;
				++matches;
			}
		}

		fclose(f);

		if( matches < 2 ) {
			RETURN_WITH_SET_ERROR( "page", SG_ERROR_PARSE, "/proc/vmstat" );
		}
	}
	else if ((f = fopen("/proc/stat", "r")) != NULL) {
		if (sg_f_read_line(f, line_buf, sizeof(line_buf), "page") == NULL) {
			fclose(f);
			RETURN_FROM_PREVIOUS_ERROR( "page", sg_get_error() );
		}

		fclose(f);

		if( sscanf( line_buf, "page %llu %llu", &page_stats_buf->pages_pagein, &page_stats_buf->pages_pageout ) != 2 ) {
			RETURN_WITH_SET_ERROR("page", SG_ERROR_PARSE, "page");
		}
	}
	else {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_OPEN, "/proc/stat");
	}
#elif defined(FREEBSD) || defined(DFBSD)
	size = sizeof(page_stats_buf->pages_pagein);
	if (sysctlbyname("vm.stats.vm.v_swappgsin", &page_stats_buf->pages_pagein, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_swappgsin");
	}

	size = sizeof(page_stats_buf->pages_pageout);
	if (sysctlbyname("vm.stats.vm.v_swappgsout", &page_stats_buf->pages_pageout, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_swappgsout");
	}
#elif defined(NETBSD) || defined(OPENBSD)
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;

	if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP");
	}

	page_stats_buf->pages_pagein = uvm.pgswapin;
	page_stats_buf->pages_pageout = uvm.pgswapout;
#elif defined(AIX)
	/* return code is number of structures returned */
	if(perfstat_memory_total(NULL, &mem, sizeof(perfstat_memory_total_t), 1) != 1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_SYSCTLBYNAME, "perfstat_memory_total");
	}

	page_stats_buf->pages_pagein  = mem.pgins;
	page_stats_buf->pages_pageout = mem.pgouts;
#elif defined(HPUX)
	if( pstat_getvminfo( &vminfo, sizeof(vminfo), 1, 0 ) == -1 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("page", SG_ERROR_SYSCTLBYNAME, "pstat_getswap");
	}

	page_stats_buf->pages_pagein  = vminfo.psv_spgin;
	page_stats_buf->pages_pageout = vminfo.psv_spgout;
#else
	RETURN_WITH_SET_ERROR("page", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	return SG_ERROR_NONE;
}

static sg_error
sg_get_page_stats_diff_int(sg_page_stats *tgt, const sg_page_stats *now, const sg_page_stats *last){

	if( NULL == tgt ) {
		RETURN_WITH_SET_ERROR("page", SG_ERROR_INVALID_ARGUMENT, "sg_get_page_stats_diff_int(tgt)");
	}

	if( now ) {
		*tgt = *now;
		if( last ) {
			tgt->pages_pagein -= last->pages_pagein;
			tgt->pages_pageout -= last->pages_pageout;
			tgt->systime -= last->systime;
		}
	}
	else {
		memset(tgt, 0, sizeof(*tgt));
	}

	return SG_ERROR_NONE;
}

EASY_COMP_SETUP(page,2,NULL);

VECTOR_INIT_INFO_EMPTY_INIT(sg_page_stats);

#define SG_PAGE_CUR_IDX	 0
#define SG_PAGE_DIFF_IDX	1

EASY_COMP_ACCESS(sg_get_page_stats,page,page,SG_PAGE_CUR_IDX)
EASY_COMP_DIFF(sg_get_page_stats_diff,sg_get_page_stats,page,page,SG_PAGE_DIFF_IDX,SG_PAGE_CUR_IDX)
