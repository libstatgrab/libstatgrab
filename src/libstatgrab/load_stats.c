/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
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

static sg_error sg_get_load_stats_int(sg_load_stats *load_stats_buf){

#ifdef HAVE_GETLOADAVG
	double loadav[3];
#elif defined(HPUX)
	struct pst_dynamic pstat_dynamic;
#elif defined(AIX)
	perfstat_cpu_total_t all_cpu_info;
	int rc;
#elif defined(SOLARIS) && !defined(HAVE_SYS_LOADAVG_H)
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;
#endif

	load_stats_buf->min1 = load_stats_buf->min5 = load_stats_buf->min15 = 0;

#ifdef HAVE_GETLOADAVG
	getloadavg(loadav,3);

	load_stats_buf->min1=loadav[0];
	load_stats_buf->min5=loadav[1];
	load_stats_buf->min15=loadav[2];
#elif defined(SOLARIS)
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_OPEN, NULL);
	}

	if((ksp=kstat_lookup(kc, "unix", 0, "system_misc")) == NULL){
		kstat_close(kc);
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_LOOKUP, "unix,0,system_misc");
	}

	if (kstat_read(kc, ksp, 0) == -1) {
		kstat_close(kc);
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_READ, NULL);
	}

	kstat_close(kc);

	if((kn=kstat_data_lookup(ksp, "avenrun_1min")) == NULL) {
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_DATA_LOOKUP, "avenrun_1min");
	}
	load_stats_buf->min1 = (double)kn->value.ui32 / (double)256;

	if((kn=kstat_data_lookup(ksp, "avenrun_5min")) == NULL) {
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_DATA_LOOKUP, "avenrun_5min");
	}
	load_stats_buf->min5 = (double)kn->value.ui32 / (double)256;

	if((kn=kstat_data_lookup(ksp, "avenrun_15min")) == NULL) {
		RETURN_WITH_SET_ERROR("load", SG_ERROR_KSTAT_DATA_LOOKUP, "avenrun_15min");
	}
	load_stats_buf->min15 = (double)kn->value.ui32 / (double)256;
#elif defined(HPUX)
	if (pstat_getdynamic(&pstat_dynamic, sizeof(pstat_dynamic), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("load", SG_ERROR_PSTAT, "pstat_dynamic");
	}

	load_stats_buf->min1=pstat_dynamic.psd_avg_1_min;
	load_stats_buf->min5=pstat_dynamic.psd_avg_5_min;
	load_stats_buf->min15=pstat_dynamic.psd_avg_15_min;
#elif defined(AIX)
	rc = perfstat_cpu_total( NULL, &all_cpu_info, sizeof(all_cpu_info), 1);
	if( -1 == rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("load", SG_ERROR_PSTAT, "perfstat_cpu_total");
	}
	else {
		load_stats_buf->min1  = (double) all_cpu_info.loadavg[0] / (double)(1 << SBITS);
		load_stats_buf->min5  = (double) all_cpu_info.loadavg[1] / (double)(1 << SBITS);
		load_stats_buf->min15 = (double) all_cpu_info.loadavg[2] / (double)(1 << SBITS);
	}
#else
	RETURN_WITH_SET_ERROR("load", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	load_stats_buf->systime = time(NULL);

	return SG_ERROR_NONE;
}

EASY_COMP_SETUP(load,1,NULL);

VECTOR_INIT_INFO_EMPTY_INIT(sg_load_stats);

#define SG_LOAD_CUR_IDX 0

EASY_COMP_ACCESS(sg_get_load_stats,load,load,SG_LOAD_CUR_IDX)
