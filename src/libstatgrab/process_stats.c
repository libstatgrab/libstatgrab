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
#if defined(SOLARIS) || defined(LINUX)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#endif

#ifdef SOLARIS
#include <procfs.h>
#include <limits.h>
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif
#ifdef LINUX
#include "tools.h"
#include <linux/limits.h>
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif
#ifdef FREEBSD
#include <kvm.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
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
#ifdef FREEBSD
	struct kinfo_proc *kp_stats;	
	kvm_t *kvmd = NULL;
	int procs;
#endif

#if defined(SOLARIS) || defined(LINUX)
	process_stat.sleeping=0;
        process_stat.running=0;
        process_stat.zombie=0;
        process_stat.stopped=0;
        process_stat.total=0;

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
#ifdef FREEBSD
	if((kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL)) == NULL){
		return NULL;
	}

	kp_stats=kvm_getprocs(kvmd, KERN_PROC_ALL, 0, &procs);

	while(procs--){
		if (kp_stats[procs].kp_proc.p_stat == SSLEEP) process_stat.sleeping++;	
		if (kp_stats[procs].kp_proc.p_stat == SRUN) process_stat.running++;
		if (kp_stats[procs].kp_proc.p_stat == SIDL) process_stat.running++;
		if (kp_stats[procs].kp_proc.p_stat == SZOMB) process_stat.zombie++;
		if (kp_stats[procs].kp_proc.p_stat == SSTOP) process_stat.stopped++;
	}

	kvm_close(kvmd);
#endif

	process_stat.total=process_stat.sleeping+process_stat.running+process_stat.zombie+process_stat.stopped;
	return &process_stat;
}
