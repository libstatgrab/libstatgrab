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
#include <statgrab.h>
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#endif

page_stat_t *get_page_stats(){
	static page_stat_t page_stats;
        kstat_ctl_t *kc;
        kstat_t *ksp;
        cpu_stat_t cs;
	uint_t swapin, swapout;

        if ((kc = kstat_open()) == NULL) {
                errf("kstat_open failure (%m)");
                return NULL;
        }
        for (ksp = kc->kc_chain; ksp!=NULL; ksp = ksp->ks_next) {
                if ((strcmp(ksp->ks_module, "cpu_stat")) != 0) continue;
                if (kstat_read(kc, ksp, &cs) == -1) {
                        continue;
                }

		page_stats+=cs.cpu_vminfo.pgswapin;
		page_stats+=cs.cpu_vminfo.pgswapout;

}
