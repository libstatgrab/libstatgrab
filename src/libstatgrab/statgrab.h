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

#include <sys/types.h>

/* FIXME typedefs for 32/64-bit types */
/* FIXME maybe tidy up field names? */
/* FIXME tab/space damage */
/* FIXME prefixes for util functions too */
/* FIXME comments for less obvious fields */

int sg_init(void);
int sg_drop_privileges(void);

typedef struct {
	char *os_name;
	char *os_release;
	char *os_version;
	char *platform;
	char *hostname;
	time_t uptime;
} sg_host_info;

sg_host_info *sg_get_host_info();

typedef struct {
        long long user;
        long long kernel;
        long long idle;
        long long iowait;
        long long swap;
        long long nice;
        long long total;
        time_t systime;
} sg_cpu_stats;

sg_cpu_stats *sg_get_cpu_stats();
sg_cpu_stats *sg_get_cpu_stats_diff();

typedef struct {
        float user;
        float kernel;
        float idle;
        float iowait;
        float swap;
        float nice;
	time_t time_taken;
} sg_cpu_percents;

sg_cpu_percents *sg_get_cpu_percents();

typedef struct {
	long long total;
	long long free;
	long long used;
	long long cache;
} sg_mem_stats;

sg_mem_stats *sg_get_mem_stats();

typedef struct {
	double min1;
	double min5;
	double min15;
} sg_load_stats;

sg_load_stats *sg_get_load_stats();

typedef struct {
	char *name_list;
	int num_entries;
} sg_user_stats;

sg_user_stats *sg_get_user_stats();

typedef struct {
	long long total;
	long long used;
	long long free;
} sg_swap_stats;

sg_swap_stats *sg_get_swap_stats();

typedef struct {
        char *device_name;
	char *fs_type;
        char *mnt_point;
        long long size;
        long long used;
        long long avail;
        long long total_inodes;
	long long used_inodes;
        long long free_inodes;
} sg_fs_stats;

sg_fs_stats *sg_get_fs_stats(int *entries);

typedef struct {
	char *disk_name;
	long long read_bytes;
	long long write_bytes;
	time_t systime;
} sg_disk_io_stats;

sg_disk_io_stats *sg_get_disk_io_stats(int *entries);
sg_disk_io_stats *sg_get_disk_io_stats_diff(int *entries);

typedef struct {
	char *interface_name;
	long long tx;
	long long rx;
	long long ipackets;
	long long opackets;
	long long ierrors;
	long long oerrors;
	long long collisions;
	time_t systime;
} sg_network_io_stats;

sg_network_io_stats *sg_get_network_io_stats(int *entries);
sg_network_io_stats *sg_get_network_io_stats_diff(int *entries);

typedef enum {
	SG_IFACE_DUPLEX_FULL,
	SG_IFACE_DUPLEX_HALF,
	SG_IFACE_DUPLEX_UNKNOWN
} sg_iface_duplex;

typedef struct {
	char *interface_name;
	int speed;	/* In megabits/sec */
	sg_iface_duplex dup;
	int up;
} sg_network_iface_stats;

sg_network_iface_stats *sg_get_network_iface_stats(int *entries);

typedef struct {
	long long pages_pagein;
	long long pages_pageout;
	time_t systime;
} sg_page_stats;

sg_page_stats *sg_get_page_stats();
sg_page_stats *sg_get_page_stats_diff();

typedef enum {
	SG_PROCESS_STATE_RUNNING,
	SG_PROCESS_STATE_SLEEPING,
	SG_PROCESS_STATE_STOPPED,
	SG_PROCESS_STATE_ZOMBIE,
	SG_PROCESS_STATE_UNKNOWN
} sg_process_state;

typedef struct {
	char *process_name;
	char *proctitle;

	pid_t pid;
	pid_t parent; /* Parent pid */
	pid_t pgid;   /* process id of process group leader */

	uid_t uid;
	uid_t euid;
	gid_t gid;
	gid_t egid;

	unsigned long long proc_size; /* in bytes */
	unsigned long long proc_resident; /* in bytes */
	time_t time_spent; /* time running in seconds */
	double cpu_percent;
	int nice;
	sg_process_state state;
} sg_process_stats;

sg_process_stats *sg_get_process_stats(int *entries);

typedef struct {
	int total;
	int running;
	int sleeping;
	int stopped;
	int zombie;
} sg_process_count;

sg_process_count *sg_get_process_count();

