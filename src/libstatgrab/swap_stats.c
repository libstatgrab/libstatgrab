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

#include <stdio.h>
#include "ukcprog.h"
#include "statgrab.h"
#ifdef SOLARIS
#include <sys/stat.h>
#include <sys/swap.h>
#include <unistd.h>
#endif

swap_stat_t *get_swap_stats(){

	static swap_stat_t swap_stat;

	struct anoninfo ai;
	int pagesize;

	if((pagesize=sysconf(_SC_PAGESIZE)) == -1){
		return NULL;
	}
	if (swapctl(SC_AINFO, &ai) == -1) {
		return NULL;
	}
	swap_stat.total = (long long)ai.ani_max * (long long)pagesize;
	swap_stat.used = (long long)ai.ani_resv * (long long)pagesize;
	swap_stat.free = swap_stat.total - swap_stat.used;

	return &swap_stat;

}
