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
#include "tools.h"
#include "vector.h"
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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
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
#include <string.h>
#include <paths.h>
#include <fcntl.h>
#include <limits.h>
#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
#include <kvm.h>
#include <tools.h>
#endif
#include <unistd.h>
#endif

static void proc_state_init(sg_process_stats *s) {
	s->process_name = NULL;
	s->proctitle = NULL;
}

static void proc_state_destroy(sg_process_stats *s) {
	free(s->process_name);
	free(s->proctitle);
}

sg_process_stats *sg_get_process_stats(int *entries){
	VECTOR_DECLARE_STATIC(proc_state, sg_process_stats, 64,
	                      proc_state_init, proc_state_destroy);
	int proc_state_size = 0;
	sg_process_stats *proc_state_ptr;
#ifdef ALLBSD
	int mib[4];
	size_t size;
	struct kinfo_proc *kp_stats;
	int procs, i;
	char *proctitle;
#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
	kvm_t *kvmd;
	char **args, **argsp;
	int argslen = 0;
#else
	long buflen;
	char *p;
#endif
#endif
#if defined(SOLARIS) || defined(LINUX)
        DIR *proc_dir;
        struct dirent *dir_entry;
        char filename[MAX_FILE_LENGTH];
        FILE *f;
#ifdef SOLARIS
	psinfo_t process_info;
#endif
#ifdef LINUX
	char s;
	/* If someone has a executable of 4k filename length, they deserve to get it truncated :) */
	char ps_name[4096];
	char *ptr;
	VECTOR_DECLARE_STATIC(psargs, char, 128, NULL, NULL);
	unsigned long stime, utime;
	int x;
	int fn;
	int len;
	int rc;
#endif

        if((proc_dir=opendir(PROC_LOCATION))==NULL){
                return NULL;
        }

        while((dir_entry=readdir(proc_dir))!=NULL){
                if(atoi(dir_entry->d_name) == 0) continue;

#ifdef SOLARIS
                snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/psinfo", dir_entry->d_name);
#endif
#ifdef LINUX
		snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/stat", dir_entry->d_name);
#endif
                if((f=fopen(filename, "r"))==NULL){
                        /* Open failed.. Process since vanished, or the path was too long.
                         * Ah well, move onwards to the next one */
                        continue;
                }
#ifdef SOLARIS
                fread(&process_info, sizeof(psinfo_t), 1, f);
#endif

		if (VECTOR_RESIZE(proc_state, proc_state_size + 1) < 0) {
			return NULL;
		}
		proc_state_ptr = proc_state+proc_state_size;

#ifdef SOLARIS		
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

                if(process_info.pr_lwp.pr_state==1) proc_state_ptr->state = SG_PROCESS_STATE_SLEEPING;
                if(process_info.pr_lwp.pr_state==2) proc_state_ptr->state = SG_PROCESS_STATE_RUNNING; 
                if(process_info.pr_lwp.pr_state==3) proc_state_ptr->state = SG_PROCESS_STATE_ZOMBIE; 
                if(process_info.pr_lwp.pr_state==4) proc_state_ptr->state = SG_PROCESS_STATE_STOPPED; 
                if(process_info.pr_lwp.pr_state==6) proc_state_ptr->state = SG_PROCESS_STATE_RUNNING; 
#endif
#ifdef LINUX
		x = fscanf(f, "%d %4096s %c %d %d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %d %*d %*d %*u %llu %llu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d\n", &(proc_state_ptr->pid), ps_name, &s, &(proc_state_ptr->parent), &(proc_state_ptr->pgid), &utime, &stime, &(proc_state_ptr->nice), &(proc_state_ptr->proc_size), &(proc_state_ptr->proc_resident));
		proc_state_ptr->proc_resident = proc_state_ptr->proc_resident * getpagesize();
		if(s == 'S') proc_state_ptr->state = SG_PROCESS_STATE_SLEEPING;
		if(s == 'R') proc_state_ptr->state = SG_PROCESS_STATE_RUNNING;
		if(s == 'Z') proc_state_ptr->state = SG_PROCESS_STATE_ZOMBIE;
		if(s == 'T') proc_state_ptr->state = SG_PROCESS_STATE_STOPPED;
		if(s == 'D') proc_state_ptr->state = SG_PROCESS_STATE_STOPPED;
	
		/* pa_name[0] should = '(' */
		ptr = strchr(&ps_name[1], ')');	
		if(ptr !=NULL) *ptr='\0';

		if (sg_update_string(&proc_state_ptr->process_name,
		                     &ps_name[1]) < 0) {
			return NULL;
		}

		/* Need to do cpu */
		

                fclose(f);

		/* proctitle */	
		snprintf(filename, MAX_FILE_LENGTH, "/proc/%s/cmdline", dir_entry->d_name);

                if((fn=open(filename, O_RDONLY)) == -1){
                        /* Open failed.. Process since vanished, or the path was too long.
                         * Ah well, move onwards to the next one */
                        continue;
                }

#define READ_BLOCK_SIZE 128
		len = 0;
		do {
			if (VECTOR_RESIZE(psargs, len + READ_BLOCK_SIZE) < 0) {
				return NULL;
			}
			rc = read(fn, psargs + len, READ_BLOCK_SIZE);
			if (rc > 0) {
				len += rc;
			}
		} while (rc == READ_BLOCK_SIZE);
		close(fn);

		if (rc == -1) {
			/* Read failed; move on. */
			continue;
		}

		/* Turn \0s into spaces within the command line. */
		ptr = psargs;
		for(x = 0; x < len; x++) {
			if (*ptr == '\0') *ptr = ' ';
			ptr++;
		}

		if (len == 0) {
			/* We want psargs to be NULL. */
			if (VECTOR_RESIZE(psargs, 0) < 0) {
				return NULL;
			}
		} else {
			/* Not empty, so append a \0. */
			if (VECTOR_RESIZE(psargs, len + 1) < 0) {
				return NULL;
			}
			psargs[len] = '\0';
		}

		if (sg_update_string(&proc_state_ptr->proctitle, psargs) < 0) {
			return NULL;
		}
#endif

		proc_state_size++;
        }
        closedir(proc_dir);
#endif

#ifdef ALLBSD
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;

	if(sysctl(mib, 3, NULL, &size, NULL, 0) < 0) {
		return NULL;
	}

	procs = size / sizeof(struct kinfo_proc);

	kp_stats = malloc(size);
	if(kp_stats == NULL) {
		return NULL;
	}
	memset(kp_stats, 0, size);

	if(sysctl(mib, 3, kp_stats, &size, NULL, 0) < 0) {
		free(kp_stats);
		return NULL;
	}

#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
	kvmd = sg_get_kvm2();
#endif

	for (i = 0; i < procs; i++) {
		const char *name;

		if (kp_stats[i].ki_stat == 0) {
			/* FreeBSD 5 deliberately overallocates the array that
			 * the sysctl returns, so we'll get a few junk
			 * processes on the end that we have to ignore. (Search
			 * for "overestimate by 5 procs" in
			 * src/sys/kern/kern_proc.c for more details.) */
			continue;
		}

		if (VECTOR_RESIZE(proc_state, proc_state_size + 1) < 0) {
			return NULL;
		}
		proc_state_ptr = proc_state+proc_state_size;

#ifdef FREEBSD5
		name = kp_stats[i].ki_comm;
#elif defined(DFBSD)
		name = kp_stats[i].kp_thread.td_comm;
#else
		name = kp_stats[i].kp_proc.p_comm;
#endif
		if (sg_update_string(&proc_state_ptr->process_name, name) < 0) {
			return NULL;
		}

#if defined(FREEBSD5) || defined(NETBSD) || defined(OPENBSD)
		size = sizeof(buflen);

#ifdef FREEBSD5
		if(sysctlbyname("kern.ps_arg_cache_limit", &buflen, &size, NULL, 0) < 0) {
			return NULL;
		}
#else
		mib[1] = KERN_ARGMAX;

		if(sysctl(mib, 2, &buflen, &size, NULL, 0) < 0) {
			return NULL;
		}
#endif

		proctitle = malloc(buflen);
		if(proctitle == NULL) {
			return NULL;
		}

		size = buflen;

#ifdef FREEBSD5
		mib[2] = KERN_PROC_ARGS;
		mib[3] = kp_stats[i].ki_pid;
#else
		mib[1] = KERN_PROC_ARGS;
		mib[2] = kp_stats[i].kp_proc.p_pid;
		mib[3] = KERN_PROC_ARGV;
#endif

		free(proc_state_ptr->proctitle);
		if(sysctl(mib, 4, proctitle, &size, NULL, 0) < 0) {
			free(proctitle);
			proc_state_ptr->proctitle = NULL;
		}
		else if(size > 0) {
			proc_state_ptr->proctitle = malloc(size+1);
			if(proc_state_ptr->proctitle == NULL) {
				return NULL;
			}
			p = proctitle;
			proc_state_ptr->proctitle[0] = '\0';
			do {
				sg_strlcat(proc_state_ptr->proctitle, p, size+1);
				sg_strlcat(proc_state_ptr->proctitle, " ", size+1);
				p += strlen(p) + 1;
			} while (p < proctitle + size);
			free(proctitle);
			/* remove trailing space */
			proc_state_ptr->proctitle[strlen(proc_state_ptr->proctitle)-1] = '\0';
		}
		else {
			free(proctitle);
			proc_state_ptr->proctitle = NULL;
		}
#else
		free(proc_state_ptr->proctitle);
		if(kvmd != NULL) {
			args = kvm_getargv(kvmd, &(kp_stats[i]), 0);
			if(args != NULL) {
				argsp = args;
				while(*argsp != NULL) {
					argslen += strlen(*argsp) + 1;
					argsp++;
				}
				proctitle = malloc(argslen + 1);
				proctitle[0] = '\0';
				if(proctitle == NULL) {
					return NULL;
				}
				while(*args != NULL) {
					sg_strlcat(proctitle, *args, argslen + 1);
					sg_strlcat(proctitle, " ", argslen + 1);
					args++;
				}
				/* remove trailing space */
				proctitle[strlen(proctitle)-1] = '\0';
				proc_state_ptr->proctitle = proctitle;
			}
			else {
				proc_state_ptr->proctitle = NULL;
			}
		}
		else {
			proc_state_ptr->proctitle = NULL;
		}
#endif

#ifdef FREEBSD5
		proc_state_ptr->pid = kp_stats[i].ki_pid;
		proc_state_ptr->parent = kp_stats[i].ki_ppid;
		proc_state_ptr->pgid = kp_stats[i].ki_pgid;
#else
		proc_state_ptr->pid = kp_stats[i].kp_proc.p_pid;
		proc_state_ptr->parent = kp_stats[i].kp_eproc.e_ppid;
		proc_state_ptr->pgid = kp_stats[i].kp_eproc.e_pgid;
#endif

#ifdef FREEBSD5
		proc_state_ptr->uid = kp_stats[i].ki_ruid;
		proc_state_ptr->euid = kp_stats[i].ki_uid;
		proc_state_ptr->gid = kp_stats[i].ki_rgid;
		proc_state_ptr->egid = kp_stats[i].ki_svgid;
#elif defined(DFBSD)
		proc_state_ptr->uid = kp_stats[i].kp_eproc.e_ucred.cr_ruid;
		proc_state_ptr->euid = kp_stats[i].kp_eproc.e_ucred.cr_svuid;
		proc_state_ptr->gid = kp_stats[i].kp_eproc.e_ucred.cr_rgid;
		proc_state_ptr->egid = kp_stats[i].kp_eproc.e_ucred.cr_svgid;
#else
		proc_state_ptr->uid = kp_stats[i].kp_eproc.e_pcred.p_ruid;
		proc_state_ptr->euid = kp_stats[i].kp_eproc.e_pcred.p_svuid;
		proc_state_ptr->gid = kp_stats[i].kp_eproc.e_pcred.p_rgid;
		proc_state_ptr->egid = kp_stats[i].kp_eproc.e_pcred.p_svgid;
#endif

#ifdef FREEBSD5
		proc_state_ptr->proc_size = kp_stats[i].ki_size;
		/* This is in pages */
		proc_state_ptr->proc_resident =
			kp_stats[i].ki_rssize * getpagesize();
		/* This is in microseconds */
		proc_state_ptr->time_spent = kp_stats[i].ki_runtime / 1000000;
		proc_state_ptr->cpu_percent =
			((double)kp_stats[i].ki_pctcpu / FSCALE) * 100.0;
		proc_state_ptr->nice = kp_stats[i].ki_nice;
#else
		proc_state_ptr->proc_size =
			kp_stats[i].kp_eproc.e_vm.vm_map.size;
		/* This is in pages */
		proc_state_ptr->proc_resident =
			kp_stats[i].kp_eproc.e_vm.vm_rssize * getpagesize();
#if defined(NETBSD) || defined(OPENBSD)
		proc_state_ptr->time_spent =
			kp_stats[i].kp_proc.p_rtime.tv_sec;
#elif defined(DFBSD)
		proc_state_ptr->time_spent = 
			( kp_stats[i].kp_thread.td_uticks +
			kp_stats[i].kp_thread.td_sticks +
			kp_stats[i].kp_thread.td_iticks ) / 1000000;
#else
		/* This is in microseconds */
		proc_state_ptr->time_spent =
			kp_stats[i].kp_proc.p_runtime / 1000000;
#endif
		proc_state_ptr->cpu_percent =
			((double)kp_stats[i].kp_proc.p_pctcpu / FSCALE) * 100.0;
		proc_state_ptr->nice = kp_stats[i].kp_proc.p_nice;
#endif

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
			proc_state_ptr->state = SG_PROCESS_STATE_RUNNING;
			break;
		case SSLEEP:
#ifdef SWAIT
		case SWAIT: /* FreeBSD 5 */
#endif
#ifdef SLOCK
		case SLOCK: /* FreeBSD 5 */
#endif
			proc_state_ptr->state = SG_PROCESS_STATE_SLEEPING;
			break;
		case SSTOP:
			proc_state_ptr->state = SG_PROCESS_STATE_STOPPED;
			break;
		case SZOMB:
#ifdef SDEAD
		case SDEAD: /* OpenBSD & NetBSD */
#endif
			proc_state_ptr->state = SG_PROCESS_STATE_ZOMBIE;
			break;
		default:
			proc_state_ptr->state = SG_PROCESS_STATE_UNKNOWN;
			break;
		}
		proc_state_size++;
	}

	free(kp_stats);
#endif

#ifdef CYGWIN
	return NULL;
#endif

	*entries = proc_state_size;
	return proc_state;
}

sg_process_count *sg_get_process_count() {
	static sg_process_count process_stat;
	sg_process_stats *ps;
	int ps_size, x;

	process_stat.sleeping = 0;
	process_stat.running = 0;
	process_stat.zombie = 0;
	process_stat.stopped = 0;
	process_stat.total = 0;

	ps = sg_get_process_stats(&ps_size);
	if (ps == NULL) {
		return NULL;
	}

	for(x = 0; x < ps_size; x++) {
		switch (ps->state) {
		case SG_PROCESS_STATE_RUNNING:
			process_stat.running++;
			break;
		case SG_PROCESS_STATE_SLEEPING:
			process_stat.sleeping++;
			break;
		case SG_PROCESS_STATE_STOPPED:
			process_stat.stopped++;
			break;
		case SG_PROCESS_STATE_ZOMBIE:
			process_stat.zombie++;
			break;
		default:
			/* currently no mapping for SG_PROCESS_STATE_UNKNOWN in
			 * sg_process_count */
			break;
		}
		ps++;
	}

	process_stat.total = ps_size;

	return &process_stat;
}
