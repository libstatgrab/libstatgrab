/* 
 * i-scream central monitoring system
 * http://www.i-scream.org.uk
 * Copyright (C) 2000-2002 i-scream
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
#ifdef SOLARIS
#include <unistd.h>
#include <kstat.h>
#endif
#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include "tools.h"
#endif

mem_stat_t *get_memory_stats(){

	static mem_stat_t mem_stat;

#ifdef SOLARIS
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;
	long totalmem;
	int pagesize;
#endif
#ifdef LINUX
	char *line_ptr;
	FILE *f;
#endif

#ifdef SOLARIS
	if((pagesize=sysconf(_SC_PAGESIZE)) == -1){
		return NULL;	
	}

	if((totalmem=sysconf(_SC_PHYS_PAGES)) == -1){
		return NULL;
	}

	if ((kc = kstat_open()) == NULL) {
		return NULL;
	}
	if((ksp=kstat_lookup(kc, "unix", 0, "system_pages")) == NULL){
		return NULL;
	}
	if (kstat_read(kc, ksp, 0) == -1) {
		return NULL;
	}
	if((kn=kstat_data_lookup(ksp, "freemem")) == NULL){
		return NULL;
	}
        kstat_close(kc);

	mem_stat.total = (long long)totalmem * (long long)pagesize;
	mem_stat.free = ((long long)kn->value.ul) * (long long)pagesize;
	mem_stat.used = mem_stat.total - mem_stat.free;
#endif

#ifdef LINUX
	f=fopen("/proc/meminfo", "r");
	if(f==NULL){
		return NULL;
	}

	if((line_ptr=f_read_line(f, "Mem:"))==NULL){
		fclose(f);
		return NULL;
	}

	fclose(f);

	/* Linux actually stores this as a unsigned long long, but
	 * our structures are just long longs. This shouldn't be a
	 * problem for sometime yet :) 
	 */
	if((sscanf(line_ptr,"Mem:  %lld %lld %lld %*d %*d %lld", \
		&mem_stat.total, \
		&mem_stat.used, \
		&mem_stat.free, \
		&mem_stat.cache))!=4){
			return NULL;
	}

#endif

	return &mem_stat;

}
