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
#if defined(SOLARIS) || defined(LINUX)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#endif

#include "tools.h"
#ifdef SOLARIS
#include <procfs.h>
#include <limits.h>
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif
#ifdef LINUX
#include <limits.h>
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif
#ifdef ALLBSD
#include <stdlib.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#if defined(FREEBSD) || defined(DFBSD)
#include <sys/user.h>
#else
#include <sys/proc.h>
#endif
#endif

process_stat_t *get_process_stats(){

	static process_stat_t process_stat;

#if defined(SOLARIS) || defined(LINUX)
	DIR *proc_dir; 
	struct dirent *dir_entry;
	char filename[MAX_FILE_LENGTH];
	FILE *f;
#endif
#ifdef LINUX
	char *line_ptr;
#endif
#ifdef SOLARIS
	psinfo_t process_info;
#endif
#ifdef ALLBSD
	int mib[3];
	size_t size;
	struct kinfo_proc *kp_stats;	
	int procs, i;
#endif

	process_stat.sleeping=0;
        process_stat.running=0;
        process_stat.zombie=0;
        process_stat.stopped=0;
        process_stat.total=0;

#if defined(SOLARIS) || defined(LINUX)
	if((proc_dir=opendir(PROC_LOCATION))==NULL){
		return NULL;
	}

	while((dir_entry=readdir(proc_dir))!=NULL){
		if(atoi(dir_entry->d_name) == 0) continue;

#ifdef SOLARIS
		snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/psinfo", dir_entry->d_name);
#endif
#ifdef LINUX
		snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/status", dir_entry->d_name);
#endif

		if((f=fopen(filename, "r"))==NULL){
			/* Open failed.. Process since vanished, or the path was too long. 
			 * Ah well, move onwards to the next one */
			continue;
		}

#ifdef SOLARIS
		fread(&process_info, sizeof(psinfo_t), 1, f);
		if(process_info.pr_lwp.pr_state==1) process_stat.sleeping++;
		if(process_info.pr_lwp.pr_state==2) process_stat.running++;
		if(process_info.pr_lwp.pr_state==3) process_stat.zombie++;
		if(process_info.pr_lwp.pr_state==4) process_stat.stopped++;
		if(process_info.pr_lwp.pr_state==6) process_stat.running++;
#endif
#ifdef LINUX
		if((line_ptr=f_read_line(f, "State:"))==NULL){
			fclose(f);
			continue;
		}
		if((line_ptr=strchr(line_ptr, '\t'))==NULL){
			fclose(f);
			continue;
		}
		line_ptr++;
		if(line_ptr=='\0'){
			fclose(f);
			continue;
		}

		if(*line_ptr=='S') process_stat.sleeping++;
		if(*line_ptr=='R') process_stat.running++;
		if(*line_ptr=='Z') process_stat.zombie++;
		if(*line_ptr=='T') process_stat.stopped++;
		if(*line_ptr=='D') process_stat.stopped++;
#endif	

		fclose(f);
	}
	closedir(proc_dir);
#endif
#ifdef ALLBSD
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;

	if (sysctl(mib, 3, NULL, &size, NULL, 0) < 0) {
		return NULL;
	}
        
	procs = size / sizeof(struct kinfo_proc);

	kp_stats = malloc(size);
	if (kp_stats == NULL) {
		return NULL;
	}
        
	if (sysctl(mib, 3, kp_stats, &size, NULL, 0) < 0) {
		free(kp_stats);
		return NULL;
	}

	for (i = 0; i < procs; i++) {
#ifdef FREEBSD5
		switch (kp_stats[i].ki_stat) {
#else
		switch (kp_stats[i].kp_proc.p_stat) {
#endif
		case SIDL:
		case SRUN:
#ifdef SONPROC
		case SONPROC: /* NetBSD */
#endif
			process_stat.running++;
			break;
		case SSLEEP:
#ifdef SWAIT
		case SWAIT: /* FreeBSD 5 */
#endif
#ifdef SLOCK
		case SLOCK: /* FreeBSD 5 */
#endif
			process_stat.sleeping++;
			break;
		case SSTOP:
			process_stat.stopped++;
			break;
		case SZOMB:
#ifdef SDEAD
		case SDEAD: /* OpenBSD & NetBSD */
#endif
			process_stat.zombie++;
			break;
                }
	}

	free(kp_stats);
#endif

#ifdef CYGWIN
	return NULL;
#endif

	process_stat.total=process_stat.sleeping+process_stat.running+process_stat.zombie+process_stat.stopped;

	return &process_stat;
}
