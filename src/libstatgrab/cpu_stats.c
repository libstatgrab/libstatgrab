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

#include <time.h>
#include "statgrab.h"
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#include <string.h>
#endif
#ifdef LINUX
#include <stdio.h>
#endif
#ifdef FREEBSD
#include <sys/sysctl.h>
#include <sys/dkstat.h>
#endif
#ifdef NETBSD
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/sched.h>
#endif

static cpu_states_t cpu_now;
static int cpu_now_uninit=1;

cpu_states_t *get_cpu_totals(){

#ifdef SOLARIS
        kstat_ctl_t *kc;
        kstat_t *ksp;
	cpu_stat_t cs;
#endif
#ifdef LINUX
	FILE *f;
#endif
#ifdef ALLBSD
#ifndef FREEBSD
	int mib[2];
#endif
#ifdef NETBSD
	u_int64_t cp_time[CPUSTATES];
#else
	long cp_time[CPUSTATES];
#endif
	size_t size;
#endif
        
	cpu_now.user=0;
	/* Not stored in linux or freebsd */
	cpu_now.iowait=0;
	cpu_now.kernel=0;
	cpu_now.idle=0;
	/* Not stored in linux or freebsd */
	cpu_now.swap=0;
	cpu_now.total=0;
	/* Not stored in solaris */
	cpu_now.nice=0;

#ifdef SOLARIS
        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }
        for (ksp = kc->kc_chain; ksp!=NULL; ksp = ksp->ks_next) {
                if ((strcmp(ksp->ks_module, "cpu_stat")) != 0) continue;
                if (kstat_read(kc, ksp, &cs) == -1) {
                        continue;
                }
		cpu_now.user+=(long long)cs.cpu_sysinfo.cpu[CPU_USER];
		cpu_now.iowait+=(long long)cs.cpu_sysinfo.cpu[CPU_WAIT];
		cpu_now.kernel+=(long long)cs.cpu_sysinfo.cpu[CPU_KERNEL];
		cpu_now.idle+=(long long)cs.cpu_sysinfo.cpu[CPU_IDLE];
		cpu_now.swap+=(long long)cs.cpu_sysinfo.cpu[CPU_STATES];
	}

	cpu_now.total=cpu_now.user+cpu_now.iowait+cpu_now.kernel+cpu_now.idle+cpu_now.swap;
	
        kstat_close(kc);
#endif
#ifdef LINUX
	if ((f=fopen("/proc/stat", "r" ))==NULL) {
		return NULL;
	}
	/* The very first line should be cpu */
	if((fscanf(f, "cpu %lld %lld %lld %lld", \
		&cpu_now.user, \
		&cpu_now.nice, \
		&cpu_now.kernel, \
		&cpu_now.idle)) != 4){
		fclose(f);
		return NULL;
	}

	fclose(f);

	cpu_now.total=cpu_now.user+cpu_now.nice+cpu_now.kernel+cpu_now.idle;
#endif
#ifdef ALLBSD
#ifdef FREEBSD
	size = sizeof cp_time;
	if (sysctlbyname("kern.cp_time", &cp_time, &size, NULL, 0) < 0){
		return NULL;
  	}
#else
	mib[0] = CTL_KERN;
	mib[1] = KERN_CP_TIME;
	size = sizeof cp_time;
	if (sysctl(mib, 2, &cp_time, &size, NULL, 0) < 0) {
		return NULL;
	}
#endif

	cpu_now.user=cp_time[CP_USER];
	cpu_now.nice=cp_time[CP_NICE];
	cpu_now.kernel=cp_time[CP_SYS];
	cpu_now.idle=cp_time[CP_IDLE];
	
	cpu_now.total=cpu_now.user+cpu_now.nice+cpu_now.kernel+cpu_now.idle;

#endif

	cpu_now.systime=time(NULL);
	cpu_now_uninit=0;


	return &cpu_now;
}

cpu_states_t *get_cpu_diff(){
	static cpu_states_t cpu_diff;
	cpu_states_t cpu_then, *cpu_tmp;

	if (cpu_now_uninit){
		if((cpu_tmp=get_cpu_totals())==NULL){
		/* Should get_cpu_totals fail */
			return NULL;
		}
		return cpu_tmp;
	}


        cpu_then.user=cpu_now.user;
        cpu_then.kernel=cpu_now.kernel;
        cpu_then.idle=cpu_now.idle;
        cpu_then.iowait=cpu_now.iowait;
        cpu_then.swap=cpu_now.swap;
        cpu_then.nice=cpu_now.nice;
        cpu_then.total=cpu_now.total;
        cpu_then.systime=cpu_now.systime;

	if((cpu_tmp=get_cpu_totals())==NULL){
		return NULL;
	}

	cpu_diff.user = cpu_now.user - cpu_then.user;
	cpu_diff.kernel = cpu_now.kernel - cpu_then.kernel;
	cpu_diff.idle = cpu_now.idle - cpu_then.idle;
	cpu_diff.iowait = cpu_now.iowait - cpu_then.iowait;
	cpu_diff.swap = cpu_now.swap - cpu_then.swap;
	cpu_diff.nice = cpu_now.nice - cpu_then.nice;
	cpu_diff.total = cpu_now.total - cpu_then.total;
	cpu_diff.systime = cpu_now.systime - cpu_then.systime;

	return &cpu_diff;
}

cpu_percent_t *cpu_percent_usage(){
	static cpu_percent_t cpu_usage;
	cpu_states_t *cs_ptr;

	cs_ptr=get_cpu_diff();
	if(cs_ptr==NULL){
		return NULL;
	}

	cpu_usage.user =  ((float)cs_ptr->user / (float)cs_ptr->total)*100;
	cpu_usage.kernel =  ((float)cs_ptr->kernel / (float)cs_ptr->total)*100;
	cpu_usage.idle = ((float)cs_ptr->idle / (float)cs_ptr->total)*100;
	cpu_usage.iowait = ((float)cs_ptr->iowait / (float)cs_ptr->total)*100;
	cpu_usage.swap = ((float)cs_ptr->swap / (float)cs_ptr->total)*100;
	cpu_usage.nice = ((float)cs_ptr->nice / (float)cs_ptr->total)*100;
	cpu_usage.time_taken = cs_ptr->systime;

	return &cpu_usage;

}

