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
#include <stdlib.h>
#include <unistd.h>
#include "statgrab.h"

#ifdef SOLARIS
#include <procfs.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif

process_stat_t *get_process_stats(){

	static process_stat_t process_stat;

	psinfo_t process_info;
	DIR *proc_dir; 
	struct dirent *dir_entry;
	char filename[MAX_FILE_LENGTH];
	FILE *f;

	process_stat.sleeping=0;
        process_stat.running=0;
        process_stat.zombie=0;
        process_stat.stopped=0;
        process_stat.running=0;

	if((proc_dir=opendir(PROC_LOCATION))==NULL){
		return NULL;
	}

	while((dir_entry=readdir(proc_dir))!=NULL){
		if(atoi(dir_entry->d_name) == 0) continue;
		snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/psinfo", dir_entry->d_name);

		if((f=fopen(filename, "r"))==NULL){
			/* Open failed.. Process since vanished, or the path was too long. 
			 * Ah well, move onwards to the next one */
			continue;
		}

		fread(&process_info, sizeof(psinfo_t), 1, f);
		fclose(f);

		if(process_info.pr_lwp.pr_state==1) process_stat.sleeping++;
		if(process_info.pr_lwp.pr_state==2) process_stat.running++;
		if(process_info.pr_lwp.pr_state==3) process_stat.zombie++;
		if(process_info.pr_lwp.pr_state==4) process_stat.stopped++;
		if(process_info.pr_lwp.pr_state==6) process_stat.running++;
	}
	
	closedir(proc_dir);


	process_stat.total=process_stat.sleeping+process_stat.running+process_stat.zombie+process_stat.stopped;
	return &process_stat;
}
