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
#include <stdlib.h>
#include <stdio.h>
#include "ukcprog.h"
#include <string.h>

#ifdef SOLARIS
#include <sys/mnttab.h>
#include <sys/types.h>
#include <sys/statvfs.h>

#define VALID_FS_TYPES {"ufs", "tmpfs"}

#endif

#define START_VAL 1

char *copy_string(char *orig_ptr, const char *newtext){

	/* Maybe free if not NULL, and strdup rather than realloc and strcpy? */
	orig_ptr=realloc(orig_ptr, (1+strlen(newtext)));
        if(orig_ptr==NULL){
        	return NULL;
        }
        strcpy(orig_ptr, newtext);

	return orig_ptr;
}


void init_disk_stat(int start, int end, disk_stat_t *disk_stats){

	for(disk_stats+=start; start<=end; start++){
		disk_stats->device_name=NULL;
		disk_stats->fs_type=NULL;
		disk_stats->mnt_point=NULL;
		
		disk_stats++;
	}

}

disk_stat_t *get_disk_stats(int *entries){

	static disk_stat_t *disk_stats;
	static int watermark=-1;

	char *fs_types[] = VALID_FS_TYPES;
	int x, valid_type;

	int num_disks=0;
	struct mnttab mp;
	struct statvfs fs;
	FILE *f;

	disk_stat_t *disk_ptr;

	if(watermark==-1){
		disk_stats=malloc(START_VAL * sizeof(disk_stat_t));
		if(disk_stats==NULL){
			return NULL;
		}
		watermark=START_VAL;
		init_disk_stat(0, watermark-1, disk_stats);
	}


	if ((f=fopen("/etc/mnttab", "r" ))==NULL){
		return NULL;
	}
	while((getmntent(f, &mp)) == 0){
		if ((statvfs(mp.mnt_mountp, &fs)) !=0){
			continue;
		}

		valid_type=0;
		for(x=0;x<((sizeof(fs_types))/(sizeof(char*)));x++){
			if(strcmp(mp.mnt_fstype, fs_types[x]) ==0){
				valid_type=1;
				break;
			}
		}

		if(valid_type){
			if(num_disks>watermark-1){
				disk_ptr=disk_stats;
				if((disk_stats=realloc(disk_stats, (watermark*2 * sizeof(disk_stat_t))))==NULL){
					disk_stats=disk_ptr;
					return NULL;
				}

				watermark=watermark*2;
				init_disk_stat(num_disks, watermark-1, disk_stats);
			}

			disk_ptr=disk_stats+num_disks;
	
			/* Memory leak in event of realloc failing */
			/* Maybe make this char[bigenough] and do strncpy's and put a null in the end? 
			   Downside is its a bit hungry for a lot of mounts, as MNT_MAX_SIZE woul prob be upwards
			   of a k each */
			if((disk_ptr->device_name=copy_string(disk_ptr->device_name, mp.mnt_special))==NULL){
				return NULL;
			}

			if((disk_ptr->fs_type=copy_string(disk_ptr->fs_type, mp.mnt_fstype))==NULL){
                                return NULL;
                        }
	
			if((disk_ptr->mnt_point=copy_string(disk_ptr->mnt_point, mp.mnt_mountp))==NULL){
                                return NULL;
                        }
			
			disk_ptr->size = (long long)fs.f_frsize * (long long)fs.f_blocks;
			disk_ptr->avail = (long long)fs.f_frsize * (long long)fs.f_bavail;
			disk_ptr->used = (disk_ptr->size) - ((long long)fs.f_frsize * (long long)fs.f_bfree);
		
			disk_ptr->total_inodes=(long long)fs.f_files;
			disk_ptr->used_inodes=disk_ptr->total_inodes - (long long)fs.f_ffree;
			disk_ptr->free_inodes=(long long)fs.f_favail;

			num_disks++;
		}
	}

	*entries=num_disks;	

	/* If this fails, there is very little i can do about it, so i'll ignore it :) */
	fclose(f);

	return disk_stats;

}
