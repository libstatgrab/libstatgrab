/* 
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2003 i-scream
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "statgrab.h"
#include "tools.h"
#include <time.h>
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#include <string.h>
#endif
#ifdef LINUX
#include <stdio.h>
#endif
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

static page_stat_t page_stats;
static int page_stats_uninit=1;

page_stat_t *get_page_stats(){
#ifdef SOLARIS
        kstat_ctl_t *kc;
        kstat_t *ksp;
        cpu_stat_t cs;
#endif
#ifdef LINUX
	FILE *f;
	char *line_ptr;
#endif
#ifdef FREEBSD
	size_t size;
#endif
#ifdef NETBSD
	struct uvmexp *uvm;
#endif

	page_stats.systime = time(NULL);
        page_stats.pages_pagein=0;
        page_stats.pages_pageout=0;

#ifdef SOLARIS
        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }
        for (ksp = kc->kc_chain; ksp!=NULL; ksp = ksp->ks_next) {
                if ((strcmp(ksp->ks_module, "cpu_stat")) != 0) continue;
                if (kstat_read(kc, ksp, &cs) == -1) {
                        continue;
                }

		page_stats.pages_pagein+=(long long)cs.cpu_vminfo.pgpgin;
		page_stats.pages_pageout+=(long long)cs.cpu_vminfo.pgpgout;
	}

	kstat_close(kc);
#endif
#ifdef LINUX
	if((f=fopen("/proc/stat", "r"))==NULL){
		return NULL;
	}
	if((line_ptr=f_read_line(f, "page"))==NULL){
		fclose(f);
		return NULL;
	}
	if((sscanf(line_ptr, "page %lld %lld", &page_stats.pages_pagein, &page_stats.pages_pageout))!=2){
		return NULL;
	}
	fclose(f);

#endif
#ifdef FREEBSD
	size = sizeof page_stats.pages_pagein;
        if (sysctlbyname("vm.stats.vm.v_swappgsin", &page_stats.pages_pagein, &size, NULL, 0) < 0){
                return NULL;
        }
	size = sizeof page_stats.pages_pageout;
        if (sysctlbyname("vm.stats.vm.v_swappgsout", &page_stats.pages_pageout, &size, NULL, 0) < 0){
                return NULL;
        }
#endif
#ifdef NETBSD
	if ((uvm = get_uvmexp()) == NULL) {
		return NULL;
	}
	page_stats.pages_pagein = uvm->pgswapin;
	page_stats.pages_pageout = uvm->pgswapout;
#endif

	return &page_stats;
}

page_stat_t *get_page_stats_diff(){
	page_stat_t *page_ptr;
	static page_stat_t page_stats_diff;

	if(page_stats_uninit){
		page_ptr=get_page_stats();
		if(page_ptr==NULL){
			return NULL;
		}
		page_stats_uninit=0;
		return page_ptr;
	}

	page_stats_diff.pages_pagein=page_stats.pages_pagein;
	page_stats_diff.pages_pageout=page_stats.pages_pageout;
	page_stats_diff.systime=page_stats.systime;

	page_ptr=get_page_stats();
	if(page_ptr==NULL){
		return NULL;
	}

        page_stats_diff.pages_pagein=page_stats.pages_pagein-page_stats_diff.pages_pagein;
        page_stats_diff.pages_pageout=page_stats.pages_pageout-page_stats_diff.pages_pageout;
        page_stats_diff.systime=page_stats.systime-page_stats_diff.systime;
	
	return &page_stats_diff;
}
