/*
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "statgrab.h"
#include "tools.h"
#ifdef SOLARIS
#include <sys/stat.h>
#include <sys/swap.h>
#include <unistd.h>
#endif
#if defined(LINUX) || defined(CYGWIN)
#include <stdio.h>
#include <string.h>
#endif
#ifdef FREEBSD
#include <unistd.h>
#include <sys/types.h>
#include <kvm.h>
#endif
#if defined(NETBSD) || defined(OPENBSD)
#include <sys/param.h>
#include <sys/time.h>
#include <uvm/uvm.h>
#endif

swap_stat_t *get_swap_stats(){

	static swap_stat_t swap_stat;

#ifdef SOLARIS
	struct anoninfo ai;
	int pagesize;
#endif
#if defined(LINUX) || defined(CYGWIN)
	FILE *f;
	char *line_ptr;
	unsigned long long value;
#endif
#ifdef FREEBSD
	struct kvm_swap swapinfo;
	int pagesize;
	kvm_t *kvmd;
#endif
#if defined(NETBSD) || defined(OPENBSD)
	struct uvmexp *uvm;
#endif

#ifdef SOLARIS
	if((pagesize=sysconf(_SC_PAGESIZE)) == -1){
		return NULL;
	}
	if (swapctl(SC_AINFO, &ai) == -1) {
		return NULL;
	}
	swap_stat.total = (long long)ai.ani_max * (long long)pagesize;
	swap_stat.used = (long long)ai.ani_resv * (long long)pagesize;
	swap_stat.free = swap_stat.total - swap_stat.used;
#endif
#if defined(LINUX) || defined(CYGWIN)
	if ((f = fopen("/proc/meminfo", "r")) == NULL) {
		return NULL;
	}

	while ((line_ptr = f_read_line(f, "")) != NULL) {
		if (sscanf(line_ptr, "%*s %llu kB", &value) != 1) {
			continue;
		}
		value *= 1024;

		if (strncmp(line_ptr, "SwapTotal:", 10) == 0) {
			swap_stat.total = value;
		} else if (strncmp(line_ptr, "SwapFree:", 9) == 0) {
			swap_stat.free = value;
		}
	}

	fclose(f);
	swap_stat.used = swap_stat.total - swap_stat.free;
#endif
#ifdef FREEBSD
	if((kvmd = get_kvm()) == NULL){
		return NULL;
	}
	if ((kvm_getswapinfo(kvmd, &swapinfo, 1,0)) == -1){
		return NULL;
	}
	pagesize=getpagesize();

	swap_stat.total= (long long)swapinfo.ksw_total * (long long)pagesize;
	swap_stat.used = (long long)swapinfo.ksw_used * (long long)pagesize;
	swap_stat.free = swap_stat.total-swap_stat.used;
#endif
#if defined(NETBSD) || defined(OPENBSD)
	if ((uvm = get_uvmexp()) == NULL) {
		return NULL;
	}

	swap_stat.total = (long long)uvm->pagesize * (long long)uvm->swpages;
	swap_stat.used = (long long)uvm->pagesize * (long long)uvm->swpginuse;
	swap_stat.free = swap_stat.total - swap_stat.used;
#endif

	return &swap_stat;

}
