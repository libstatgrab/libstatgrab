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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "statgrab.h"

#ifdef SOLARIS
#include <sys/mnttab.h>
#include <sys/statvfs.h>
#include <kstat.h>
#define VALID_FS_TYPES {"ufs", "tmpfs"}
#endif

#ifdef LINUX
#include <time.h>
#include <sys/vfs.h>
#include <mntent.h>
#include "tools.h"
#define VALID_FS_TYPES {"ext2", "ext3", "xfs", "reiserfs", "vfat", "tmpfs"}
#endif

#ifdef FREEBSD
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/dkstat.h>
#include <devstat.h>
#define VALID_FS_TYPES {"ufs", "mfs"}
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
#if defined(LINUX) || defined (SOLARIS)
	FILE *f;
#endif

	disk_stat_t *disk_ptr;

#ifdef SOLARIS
	struct mnttab mp;
	struct statvfs fs;
#endif
#ifdef LINUX
	struct mntent *mp;
	struct statfs fs;
#endif
#ifdef FREEBSD
	int nummnt;
	struct statfs *mp;
#endif

	if(watermark==-1){
		disk_stats=malloc(START_VAL * sizeof(disk_stat_t));
		if(disk_stats==NULL){
			return NULL;
		}
		watermark=START_VAL;
		init_disk_stat(0, watermark-1, disk_stats);
	}
#ifdef FREEBSD
	nummnt=getmntinfo(&mp , MNT_LOCAL);
	if (nummnt<=0){
		return NULL;
	}
	for(;nummnt--; mp++){
		valid_type=0;
		for(x=0;x<((sizeof(fs_types))/(sizeof(char*)));x++){
                        if(strcmp(mp->f_fstypename, fs_types[x]) ==0){
                                valid_type=1;
                                break;
                        }
                }
#endif

#ifdef LINUX
	if ((f=setmntent("/etc/mtab", "r" ))==NULL){
		return NULL;
	}

	while((mp=getmntent(f))){
		if((statfs(mp->mnt_dir, &fs)) !=0){
			continue;
		}	

		valid_type=0;
		for(x=0;x<((sizeof(fs_types))/(sizeof(char*)));x++){
                        if(strcmp(mp->mnt_type, fs_types[x]) ==0){
                                valid_type=1;
                                break;
                        }
                }
#endif

#ifdef SOLARIS
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
#endif

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
#ifdef FREEBSD
			if((disk_ptr->device_name=copy_string(disk_ptr->device_name, mp->f_mntfromname))==NULL){
				return NULL;
			}

			if((disk_ptr->fs_type=copy_string(disk_ptr->fs_type, mp->f_fstypename))==NULL){
				return NULL;
			}

			if((disk_ptr->mnt_point=copy_string(disk_ptr->mnt_point, mp->f_mntonname))==NULL){
				return NULL;
			}

			disk_ptr->size = (long long)mp->f_bsize * (long long) mp->f_blocks;
			disk_ptr->avail = (long long)mp->f_bsize * (long long) mp->f_bavail;
			disk_ptr->used = (disk_ptr->size) - ((long long)mp->f_bsize * (long long)mp->f_bfree);

			disk_ptr->total_inodes=(long long)mp->f_files;
			disk_ptr->free_inodes=(long long)mp->f_ffree;
			/* Freebsd doesn't have a "available" inodes */
			disk_ptr->used_inodes=disk_ptr->total_inodes-disk_ptr->free_inodes;
#endif
#ifdef LINUX
			if((disk_ptr->device_name=copy_string(disk_ptr->device_name, mp->mnt_fsname))==NULL){
				return NULL;
			}
				
			if((disk_ptr->fs_type=copy_string(disk_ptr->fs_type, mp->mnt_type))==NULL){	
				return NULL;
			}

			if((disk_ptr->mnt_point=copy_string(disk_ptr->mnt_point, mp->mnt_dir))==NULL){
				return NULL;
			}
			disk_ptr->size = (long long)fs.f_bsize * (long long)fs.f_blocks;
			disk_ptr->avail = (long long)fs.f_bsize * (long long)fs.f_bavail;
			disk_ptr->used = (disk_ptr->size) - ((long long)fs.f_bsize * (long long)fs.f_bfree);

			disk_ptr->total_inodes=(long long)fs.f_files;
			disk_ptr->free_inodes=(long long)fs.f_ffree;
			/* Linux doesn't have a "available" inodes */
			disk_ptr->used_inodes=disk_ptr->total_inodes-disk_ptr->free_inodes;
#endif

#ifdef SOLARIS
			/* Memory leak in event of realloc failing */
			/* Maybe make this char[bigenough] and do strncpy's and put a null in the end? 
			 * Downside is its a bit hungry for a lot of mounts, as MNT_MAX_SIZE would prob 
			 * be upwards of a k each 
			 */
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
#endif
			num_disks++;
		}
	}

	*entries=num_disks;	

	/* If this fails, there is very little i can do about it, so i'll ignore it :) */
#if defined(LINUX) || defined(SOLARIS)
	fclose(f);
#endif

	return disk_stats;

}
void diskio_stat_init(int start, int end, diskio_stat_t *diskio_stats){

	for(diskio_stats+=start; start<end; start++){
		diskio_stats->disk_name=NULL;
		
		diskio_stats++;
	}
}

diskio_stat_t *diskio_stat_malloc(int needed_entries, int *cur_entries, diskio_stat_t *diskio_stats){

        if(diskio_stats==NULL){

                if((diskio_stats=malloc(needed_entries * sizeof(diskio_stat_t)))==NULL){
                        return NULL;
                }
                diskio_stat_init(0, needed_entries, diskio_stats);
                *cur_entries=needed_entries;

                return diskio_stats;
        }


        if(*cur_entries<needed_entries){
                diskio_stats=realloc(diskio_stats, (sizeof(diskio_stat_t)*needed_entries));
                if(diskio_stats==NULL){
                        return NULL;
                }
                diskio_stat_init(*cur_entries, needed_entries, diskio_stats);
                *cur_entries=needed_entries;
        }

        return diskio_stats;
}

static diskio_stat_t *diskio_stats=NULL;	
static int num_diskio=0;	

#ifdef LINUX
typedef struct {
	int major;
	int minor;
} partition;
#endif

diskio_stat_t *get_diskio_stats(int *entries){

	static int sizeof_diskio_stats=0;
	diskio_stat_t *diskio_stats_ptr;

#ifdef SOLARIS
        kstat_ctl_t *kc;
        kstat_t *ksp;
	kstat_io_t kios;
#endif
#ifdef LINUX
	FILE *f;
	char *line_ptr;
	int major, minor;
	char dev_letter;
	int has_pp_stats = 1;
	static partition *parts = NULL;
	static int alloc_parts = 0;
	int i, n;
	time_t now;
#endif
#ifdef FREEBSD
	static struct statinfo stats;
	static int stats_init = 0;
	int counter;
	struct device_selection *dev_sel = NULL;
	int n_selected, n_selections;
	long sel_gen;
	struct devstat *dev_ptr;
#endif
	num_diskio=0;

#ifdef FREEBSD
	if (!stats_init) {
		stats.dinfo=malloc(sizeof(struct devinfo));
		bzero(stats.dinfo, sizeof(struct devinfo));
		if(stats.dinfo==NULL) return NULL;
		stats_init = 1;
	}
#ifdef FREEBSD5
	if ((devstat_getdevs(NULL, &stats)) < 0) return NULL;
	/* Not aware of a get all devices, so i said 999. If we ever
	 * find a machine with more than 999 disks, then i'll change
	 * this number :)
	 */
	if (devstat_selectdevs(&dev_sel, &n_selected, &n_selections, &sel_gen, stats.dinfo->generation, stats.dinfo->devices, stats.dinfo->numdevs, NULL, 0, NULL, 0, DS_SELECT_ONLY, 999, 1) < 0) return NULL;
#else
	if ((getdevs(&stats)) < 0) return NULL;
	/* Not aware of a get all devices, so i said 999. If we ever
	 * find a machine with more than 999 disks, then i'll change
	 * this number :)
	 */
	if (selectdevs(&dev_sel, &n_selected, &n_selections, &sel_gen, stats.dinfo->generation, stats.dinfo->devices, stats.dinfo->numdevs, NULL, 0, NULL, 0, DS_SELECT_ONLY, 999, 1) < 0) return NULL;
#endif

	for(counter=0;counter<stats.dinfo->numdevs;counter++){
		dev_ptr=&stats.dinfo->devices[dev_sel[counter].position];

		/* Throw away devices that have done nothing, ever.. Eg "odd" 
		 * devices.. like mem, proc.. and also doesn't report floppy
		 * drives etc unless they are doing stuff :)
		 */
#ifdef FREEBSD5
		if((dev_ptr->bytes[DEVSTAT_READ]==0) && (dev_ptr->bytes[DEVSTAT_WRITE]==0)) continue;
#else
		if((dev_ptr->bytes_read==0) && (dev_ptr->bytes_written==0)) continue;
#endif
		if((diskio_stats=diskio_stat_malloc(num_diskio+1, &sizeof_diskio_stats, diskio_stats))==NULL){
			return NULL;
		}
		diskio_stats_ptr=diskio_stats+num_diskio;

#ifdef FREEBSD5		
		diskio_stats_ptr->read_bytes=dev_ptr->bytes[DEVSTAT_READ];
		diskio_stats_ptr->write_bytes=dev_ptr->bytes[DEVSTAT_WRITE];
#else
		diskio_stats_ptr->read_bytes=dev_ptr->bytes_read;
		diskio_stats_ptr->write_bytes=dev_ptr->bytes_written;
#endif
		if(diskio_stats_ptr->disk_name!=NULL) free(diskio_stats_ptr->disk_name);
		asprintf((&diskio_stats_ptr->disk_name), "%s%d", dev_ptr->device_name, dev_ptr->unit_number);
		diskio_stats_ptr->systime=time(NULL);

		num_diskio++;
	}
	free(dev_sel);

#endif
#ifdef SOLARIS
	if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
                if (!strcmp(ksp->ks_class, "disk")) {

			if(ksp->ks_type != KSTAT_TYPE_IO) continue;
			/* We dont want metadevices appearins as num_diskio */
			if(strcmp(ksp->ks_module, "md")==0) continue;
                        if((kstat_read(kc, ksp, &kios))==-1){	
			}
			
			if((diskio_stats=diskio_stat_malloc(num_diskio+1, &sizeof_diskio_stats, diskio_stats))==NULL){
				kstat_close(kc);
				return NULL;
			}
			diskio_stats_ptr=diskio_stats+num_diskio;
			
			diskio_stats_ptr->read_bytes=kios.nread;
			
			diskio_stats_ptr->write_bytes=kios.nwritten;

			if(diskio_stats_ptr->disk_name!=NULL) free(diskio_stats_ptr->disk_name);

			diskio_stats_ptr->disk_name=strdup(ksp->ks_name);
			diskio_stats_ptr->systime=time(NULL);
			num_diskio++;
		}
	}

	kstat_close(kc);
#endif

#ifdef LINUX
	num_diskio = 0;
	n = 0;

	/* Read /proc/partitions to find what devices exist. Recent 2.4 kernels
	   have statistics in here too, so we can use those directly. */

	f = fopen("/proc/partitions", "r");
	if (f == NULL) goto out;
	now = time(NULL);

	while ((line_ptr = f_read_line(f, "")) != NULL) {
		char name[20];
		char *s;
		long long rsect, wsect;

		int nr = sscanf(line_ptr,
			" %d %d %*d %19s %*d %*d %lld %*d %*d %*d %lld",
			&major, &minor, name, &rsect, &wsect);
		if (nr < 3) continue;
		if (nr < 5) {
			has_pp_stats = 0;
			rsect = 0;
			wsect = 0;
		}

		/* Skip device names ending in numbers, since they're
		   partitions. */
		s = name;
		while (*s != '\0') s++;
		--s;
		if (*s >= '0' && *s <= '9') continue;

		diskio_stats = diskio_stat_malloc(n + 1, &sizeof_diskio_stats,
			diskio_stats);
		if (diskio_stats == NULL) goto out;
		if (n >= alloc_parts) {
			alloc_parts += 16;
			parts = realloc(parts, alloc_parts * sizeof *parts);
			if (parts == NULL) {
				alloc_parts = 0;
				goto out;
			}
		}

		if (diskio_stats[n].disk_name != NULL)
			free(diskio_stats[n].disk_name);
		diskio_stats[n].disk_name = strdup(name);
		diskio_stats[n].read_bytes = rsect * 512;
		diskio_stats[n].write_bytes = wsect * 512;
		diskio_stats[n].systime = now;
		parts[n].major = major;
		parts[n].minor = minor;

		n++;
	}

	if (!has_pp_stats) {
		/* This is an older kernel without stats in /proc/partitions.
		   Read what we can from /proc/stat instead. */

		f = fopen("/proc/stat", "r");
		if (f == NULL) goto out;
		now = time(NULL);
	
		line_ptr = f_read_line(f, "disk_io:");
		if (line_ptr == NULL) goto out;
	
		while((line_ptr=strchr(line_ptr, ' '))!=NULL){
			long long rsect, wsect;

			if (*++line_ptr == '\0') break;
	
			if((sscanf(line_ptr,
				"(%d,%d):(%*d, %*d, %lld, %*d, %lld)",
				&major, &minor, &rsect, &wsect)) != 4) {
					continue;
			}

			/* Find the corresponding device from earlier.
			   Just to add to the fun, "minor" is actually the disk
			   number, not the device minor, so we need to figure
			   out the real minor number based on the major!
			   This list is not exhaustive; if you're running
			   an older kernel you probably don't have fancy
			   I2O hardware anyway... */
			switch (major) {
			case 3:
			case 21:
			case 22:
			case 33:
			case 34:
			case 36:
			case 56:
			case 57:
			case 88:
			case 89:
			case 90:
			case 91:
				minor *= 64;
				break;
			case 9:
			case 43:
				break;
			default:
				minor *= 16;
				break;
			}
			for (i = 0; i < n; i++) {
				if (major == parts[i].major
					&& minor == parts[i].minor)
					break;
			}
			if (i == n) continue;

			/* We read the number of blocks. Blocks are stored in
			   512 bytes */
			diskio_stats[i].read_bytes = rsect * 512;
			diskio_stats[i].write_bytes = wsect * 512;
			diskio_stats[i].systime = now;
		}
	}

	num_diskio = n;
out:
	if (f != NULL) fclose(f);

#endif
	*entries=num_diskio;

	return diskio_stats;
}

diskio_stat_t *get_diskio_stats_diff(int *entries){
	static diskio_stat_t *diskio_stats_diff=NULL;
	static int sizeof_diskio_stats_diff=0;
	diskio_stat_t *diskio_stats_diff_ptr, *diskio_stats_ptr;
	int disks, x, y;

	if(diskio_stats==NULL){
		diskio_stats_ptr=get_diskio_stats(&disks);
		*entries=disks;
		return diskio_stats_ptr;
	}

	diskio_stats_diff=diskio_stat_malloc(num_diskio, &sizeof_diskio_stats_diff, diskio_stats_diff);
	if(diskio_stats_diff==NULL){
		return NULL;
	}

	diskio_stats_diff_ptr=diskio_stats_diff;
	diskio_stats_ptr=diskio_stats;

	for(disks=0;disks<num_diskio;disks++){
		if(diskio_stats_diff_ptr->disk_name!=NULL){
			free(diskio_stats_diff_ptr->disk_name);
		}
		diskio_stats_diff_ptr->disk_name=strdup(diskio_stats_ptr->disk_name);
		diskio_stats_diff_ptr->read_bytes=diskio_stats_ptr->read_bytes;
		diskio_stats_diff_ptr->write_bytes=diskio_stats_ptr->write_bytes;
		diskio_stats_diff_ptr->systime=diskio_stats_ptr->systime;

		diskio_stats_diff_ptr++;
		diskio_stats_ptr++;
	}

	diskio_stats_ptr=get_diskio_stats(&disks);
	diskio_stats_diff_ptr=diskio_stats_diff;

	for(x=0;x<sizeof_diskio_stats_diff;x++){

		if((strcmp(diskio_stats_diff_ptr->disk_name, diskio_stats_ptr->disk_name))==0){
			diskio_stats_diff_ptr->read_bytes=diskio_stats_ptr->read_bytes-diskio_stats_diff_ptr->read_bytes;
			diskio_stats_diff_ptr->write_bytes=diskio_stats_ptr->write_bytes-diskio_stats_diff_ptr->write_bytes;
			diskio_stats_diff_ptr->systime=diskio_stats_ptr->systime-diskio_stats_diff_ptr->systime;
		}else{
			diskio_stats_ptr=diskio_stats;
			for(y=0;y<disks;y++){
				if((strcmp(diskio_stats_diff_ptr->disk_name, diskio_stats_ptr->disk_name))==0){
					diskio_stats_diff_ptr->read_bytes=diskio_stats_ptr->read_bytes-diskio_stats_diff_ptr->read_bytes;
					diskio_stats_diff_ptr->write_bytes=diskio_stats_ptr->write_bytes-diskio_stats_diff_ptr->write_bytes;
					diskio_stats_diff_ptr->systime=diskio_stats_ptr->systime-diskio_stats_diff_ptr->systime;

					break;
				}
				
				diskio_stats_ptr++;
			}
		}

		diskio_stats_ptr++;
		diskio_stats_diff_ptr++;	

	}
	
	*entries=sizeof_diskio_stats_diff;
	return diskio_stats_diff;
}
