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

int get_proc_snapshot(proc_state_t **ps){
	proc_state_t *proc_state = NULL;
	proc_state_t *proc_state_ptr;
	int proc_state_size = 0;
#if defined(SOLARIS) || defined(LINUX)
        DIR *proc_dir;
        struct dirent *dir_entry;
        char filename[MAX_FILE_LENGTH];
        FILE *f;
#ifdef SOLARIS
	psinfo_t process_info;
#endif

        if((proc_dir=opendir(PROC_LOCATION))==NULL){
                return NULL;
        }

        while((dir_entry=readdir(proc_dir))!=NULL){
                if(atoi(dir_entry->d_name) == 0) continue;

#ifdef SOLARIS
                snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/psinfo", dir_entry->d_name);
#endif
                if((f=fopen(filename, "r"))==NULL){
                        /* Open failed.. Process since vanished, or the path was too long.
                         * Ah well, move onwards to the next one */
                        continue;
                }
#ifdef SOLARIS
                fread(&process_info, sizeof(psinfo_t), 1, f);

		proc_state = realloc(proc_state, (1+proc_state_size)*sizeof(proc_state_t));
		proc_state_ptr = proc_state+proc_state_size;
		
		proc_state_ptr->pid = process_info.pr_pid;
		proc_state_ptr->parent = process_info.pr_ppid;
		proc_state_ptr->pgid = process_info.pr_pgid;
		proc_state_ptr->uid = process_info.pr_uid;
		proc_state_ptr->euid = process_info.pr_euid;
		proc_state_ptr->gid = process_info.pr_gid;
		proc_state_ptr->egid = process_info.pr_egid;
		proc_state_ptr->proc_size = (process_info.pr_size) * 1024;
		proc_state_ptr->proc_resident = (process_info.pr_rssize) * 1024;
		proc_state_ptr->time_spent = process_info.pr_time.tv_sec;
		proc_state_ptr->cpu_percent = (process_info.pr_pctcpu * 100.0) / 0x8000;
		proc_state_ptr->process_name = strdup(process_info.pr_fname);
		proc_state_ptr->proctitle = strdup(process_info.pr_psargs);

                if(process_info.pr_lwp.pr_state==1) proc_state_ptr->state = SLEEPING;
                if(process_info.pr_lwp.pr_state==2) proc_state_ptr->state = RUNNING; 
                if(process_info.pr_lwp.pr_state==3) proc_state_ptr->state = ZOMBIE; 
                if(process_info.pr_lwp.pr_state==4) proc_state_ptr->state = STOPPED; 
                if(process_info.pr_lwp.pr_state==6) proc_state_ptr->state = RUNNING; 
#endif

		proc_state_size++;
#endif


                fclose(f);
        }
        closedir(proc_dir);
	*ps = proc_state;

	return proc_state_size;
}

