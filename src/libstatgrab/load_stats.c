/*
 * i-scream libstatgrab
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

#include <stdio.h>
#include <stdlib.h>
#include "statgrab.h"
#ifdef SOLARIS
#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#else
#include <kstat.h>
#endif
#endif

sg_load_stats *sg_get_load_stats(){

	static sg_load_stats load_stat;

	double loadav[3];

#ifdef CYGWIN
	return NULL;
#else

#if defined(SOLARIS) && !defined(HAVE_SYS_LOADAVG_H)

	kstat_ctl_t *kc;	
        kstat_t *ksp;
        kstat_named_t *kn;

	if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

	if((ksp=kstat_lookup(kc, "unix", 0, "system_misc")) == NULL){
                return NULL;
        }

        if (kstat_read(kc, ksp, 0) == -1) {
                return NULL;
        }

	if((kn=kstat_data_lookup(ksp, "avenrun_1min")) == NULL){
                return NULL;
        }
	load_stat.min1 = (double)kn->value.ui32 / (double)256;

        if((kn=kstat_data_lookup(ksp, "avenrun_5min")) == NULL){
                return NULL;
        }
        load_stat.min5 = (double)kn->value.ui32 / (double)256;

        if((kn=kstat_data_lookup(ksp, "avenrun_15min")) == NULL){
                return NULL;
        }
        load_stat.min15 = (double)kn->value.ui32 / (double)256;
#else
	getloadavg(loadav,3);

	load_stat.min1=loadav[0];
	load_stat.min5=loadav[1];
	load_stat.min15=loadav[2];
#endif

	return &load_stat;
#endif
}
