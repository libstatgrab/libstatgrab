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
#include <sys/utsname.h>
#include "statgrab.h"
#ifdef SOLARIS
#include <kstat.h>
#include <time.h>
#endif

general_stat_t *get_general_stats(){

	static general_stat_t general_stat;	

	struct utsname os;
	time_t boottime,curtime;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_named_t *kn;

	if((uname(&os)) < 0){
		return NULL;
	}
	
	general_stat.os_name = os.sysname;
        general_stat.os_release = os.release;
        general_stat.os_version = os.version;
        general_stat.platform = os.machine;
        general_stat.hostname = os.nodename;

	/* get uptime */
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

	return &general_stat;
	
}
