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

#include <sys/utsname.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef SOLARIS
#include <kstat.h>
#include <time.h>
#endif
#if defined(LINUX) || defined(CYGWIN)
#include <stdio.h>
#endif
#ifdef ALLBSD
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <sys/param.h>
#include <sys/sysctl.h>
#endif
#include <time.h>
#include <sys/time.h>
#endif

general_stat_t *get_general_stats(){

	static general_stat_t general_stat;	
	static struct utsname os;

#ifdef SOLARIS
	time_t boottime,curtime;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;
#endif
#if defined(LINUX) || defined(CYGWIN)
	FILE *f;
#endif
#ifdef ALLBSD
	int mib[2];
	struct timeval boottime;
	time_t curtime;
	size_t size;
#endif

	if((uname(&os)) < 0){
		return NULL;
	}
	
	general_stat.os_name = os.sysname;
        general_stat.os_release = os.release;
        general_stat.os_version = os.version;
        general_stat.platform = os.machine;
        general_stat.hostname = os.nodename;

	/* get uptime */
#ifdef SOLARIS
	if ((kc = kstat_open()) == NULL) {
		return NULL;
	}
	if((ksp=kstat_lookup(kc, "unix", -1, "system_misc"))==NULL){
		return NULL;
	}
	if (kstat_read(kc, ksp, 0) == -1) {
		return NULL;
	}
	if((kn=kstat_data_lookup(ksp, "boot_time")) == NULL){
		return NULL;
	}
	boottime=(kn->value.ui32);

	kstat_close(kc);

	time(&curtime);
	general_stat.uptime = curtime - boottime;
#endif
#if defined(LINUX) || defined(CYGWIN)
	if ((f=fopen("/proc/uptime", "r")) == NULL) {
		return NULL;
	}
	if((fscanf(f,"%lu %*d",&general_stat.uptime)) != 1){
		return NULL;
	}
	fclose(f);
#endif
#ifdef ALLBSD
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof boottime;
	if (sysctl(mib, 2, &boottime, &size, NULL, 0) < 0){
		return NULL;
	}
	time(&curtime);
	general_stat.uptime=curtime-boottime.tv_sec;
#endif

	return &general_stat;
	
}
