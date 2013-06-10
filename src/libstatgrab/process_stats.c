/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
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

#define __NEED_SG_GET_SYS_PAGE_SIZE
#include "tools.h"

#ifdef HAVE_PROCFS
/* XXX move it to a ./configure --with-proc-location[=/proc] setting */
#define PROC_LOCATION "/proc"
#define MAX_FILE_LENGTH PATH_MAX
#endif

#if defined(AIX)
# ifndef HAVE_DECL_GETPROCS64
extern int getprocs64(struct procentry64 *, int, struct fdsinfo64 *, int, pid_t *, int);
# endif
# ifndef HAVE_DECL_GETARGS
extern int getargs(struct procentry64 *, int, char *, int);
# endif
#endif

static void sg_process_stats_item_init(sg_process_stats *d) {

	d->process_name = NULL;
	d->proctitle = NULL;
}

#if 0
static sg_error sg_process_stats_item_copy(sg_process_stats *d, const sg_process_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->process_name, s->process_name) ||
	    SG_ERROR_NONE != sg_update_string(&d->proctitle, s->proctitle) ) {
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

	d->pid = s->pid;
	d->parent = s->parent;
	d->pgid = s->pgid;
	d->uid = s->uid;
	d->euid = s->euid;
	d->gid = s->gid;
	d->egid = s->egid;
	d->proc_size = s->proc_size;
	d->proc_resident = s->proc_resident;
	d->time_spent = s->time_spent;
	d->cpu_percent = s->cpu_percent;
	d->nice = s->nice;
	d->state = s->state;

	return SG_ERROR_NONE;
}

static sg_error sg_process_stats_item_compute_diff(const sg_process_stats *s, sg_process_stats *d) {

	/* XXX how "diff" user or nice changes etc. */
	d->proc_size -= s->proc_size;
	d->proc_resident -= s->proc_resident;
	d->time_spent -= s->time_spent;
	d->cpu_percent -= s->cpu_percent;

	return SG_ERROR_NONE;
}

static int sg_process_stats_item_compare(const sg_process_stats *a, const sg_process_stats *b) {

	int rc;

	if( ( 0 != ( rc = a->pid - b->pid ) )
	 || ( 0 != ( rc = a->parent - b->parent ) )
	 || ( 0 != ( rc = a->pgid - b->pgid ) )
	 || ( 0 != ( rc = strcmp(a->device_name, b->device_name) ) ) )
		return rc;

	return 0;
}
#else
#define sg_process_stats_item_copy NULL
#define sg_process_stats_item_compute_diff NULL
#define sg_process_stats_item_compare NULL
#endif

static void sg_process_stats_item_destroy(sg_process_stats *d) {

	free(d->process_name);
	free(d->proctitle);
}

VECTOR_INIT_INFO_FULL_INIT(sg_process_stats);
VECTOR_INIT_INFO_EMPTY_INIT(sg_process_count);

#define SG_PROC_STAT_IDX	0
#define SG_PROC_COUNT_IDX	1
#define SG_PROC_IDX_COUNT       2

/* kvm_openfiles() cann succeed at any later point when the admin adjust
 * permissions of accessed files - no reason to die in init() */
EASY_COMP_SETUP(process,SG_PROC_IDX_COUNT,NULL);

#ifdef HAVE_PROCFS
struct pids_in_proc_dir_t {
	size_t nitems;
	struct pids_in_proc_dir_t *next;
	pid_t items[];
};

static struct pids_in_proc_dir_t *
alloc_pids_in_proc_dir(void) {
	ssize_t pagesize = sg_get_sys_page_size();
	if( -1 != pagesize ) {
		struct pids_in_proc_dir_t *pipd = malloc( pagesize );
		if( pipd != NULL ) {
			pipd->nitems = 0;
			pipd->next = NULL;
		}

		return pipd;
	}

	return NULL;
}

/**
 * free pids_in_proc_dir_t structure
 *
 * @param pipd - pointer to structure to be freed
 * @param include_children - when true, next pointer chain is followed and freed, too
 *
 * @return next pids_in_proc_dir_t item (or NULL, when no remains)
 */
static struct pids_in_proc_dir_t *
free_pids_in_proc_dir( struct pids_in_proc_dir_t *pipd, bool include_children ) {
	struct pids_in_proc_dir_t *next = NULL;
	while( pipd != NULL ) {
		next = pipd->next;
		free(pipd);
		if( include_children )
			pipd = next;
		else
			pipd = NULL;
	}

	return next;
}

#define PIPD_IS_FULL(pipd,size) (((size) - ((offsetof(struct pids_in_proc_dir_t, items)))/sizeof(pid_t)) < ((pipd)->nitems + 1))

static struct pids_in_proc_dir_t *
add_pid_to_pids_in_proc_dir( pid_t pid, struct pids_in_proc_dir_t *pipd ) {
	assert( pipd != NULL );
	assert( NULL == pipd->next );

	if( PIPD_IS_FULL( pipd, sg_get_sys_page_size() ) ) {
		pipd->next = alloc_pids_in_proc_dir();
		if( NULL == pipd->next )
			return NULL;
		pipd = pipd->next;
	}

	pipd->items[pipd->nitems++] = pid;

	return pipd;
}

static struct pids_in_proc_dir_t *
scan_proc_dir( const char *path_to_proc_dir ) {
	DIR *proc_dir;
	struct dirent *dir_entry, *result = NULL;
	size_t dir_entry_size = sizeof(*dir_entry) - sizeof(dir_entry->d_name) + PATH_MAX + 1;
	struct pids_in_proc_dir_t *cnt = alloc_pids_in_proc_dir(), *wrk;
	int rc;

	if( NULL == cnt )
		return NULL;

	dir_entry = calloc(1, dir_entry_size);
	if( NULL == dir_entry) {
		free_pids_in_proc_dir(cnt, true);
		return NULL;
	}

	if( ( proc_dir = opendir(path_to_proc_dir) ) == NULL ) {
		SET_ERROR_WITH_ERRNO("process", SG_ERROR_OPENDIR, path_to_proc_dir);
		free_pids_in_proc_dir(cnt, true);
		free(dir_entry);
		return NULL;
	}

	wrk = cnt;
	while( ( rc = readdir_r( proc_dir, dir_entry, &result ) ) == 0 ) {
		pid_t pid;
		if( NULL == result )
			break;
		if( 0 != sscanf(dir_entry->d_name, FMT_PID_T, &pid) ) {
			wrk = add_pid_to_pids_in_proc_dir( pid, wrk );
			if( NULL == wrk ) {
				free_pids_in_proc_dir(cnt, true);
				cnt = NULL;
				break; /* bail out */
			}
		}
	}

	if( rc != 0 ) {
		SET_ERROR_WITH_ERRNO_CODE( "process", SG_ERROR_READDIR, rc, path_to_proc_dir );
	}

	free(dir_entry);
	closedir(proc_dir);

	return cnt;
}
#endif

#if defined(KERN_PROC_ARGS) || defined(KERN_PROC_ARGV) || defined(KERN_PROCARGS2) || defined(AIX) || defined(LINUX)
static char *
adjust_procname_cmndline( char *proctitle, size_t len ) {

	char *p, *pt;

	/* XXX OpenBSD prepends char *[] adressing the several embedded argv items */
	if( len && ( (size_t)((((char **)(proctitle))[0]) - proctitle) <= len ) ) {
		pt = p = ((char **)(proctitle))[0];
		len -= pt - proctitle;
	}
	else {
		pt = p = proctitle;
	}

	while( ( len && ( p < (pt + len) ) ) || !len ) {
		if( *(p+1) == '\0' )
			break;
		if( *p == '\0' ) {
			*p++ = ' ';
		}
		else {
			/* avoid overread when whitespace at end */
			p += strlen(p);
		}
	}
	if( len ) {
		pt[len] = '\0';
	}
	else {
		*p = '\0';
	}

	return pt;
}
#endif


static sg_error
sg_get_process_stats_int(sg_vector **proc_stats_vector_ptr) {

	size_t proc_items = 0;
	sg_process_stats *proc_stats_ptr;
	time_t now = time(NULL);

#if defined(LINUX)
	struct pids_in_proc_dir_t *pids_in_proc_dir;
	size_t pid_item = 0;
	char filename[MAX_FILE_LENGTH];
	FILE *f;
	char s;
	char read_buf[4096];
	char *read_ptr;
	/* XXX or we detect max command line length in ./configure */
	unsigned long stime, utime;
	unsigned long long starttime;
	int fd, rc;
	size_t len;
	time_t uptime;
	long tickspersec;
#elif defined(SOLARIS)
	struct pids_in_proc_dir_t *pids_in_proc_dir;
	size_t pid_item = 0;
	char filename[MAX_FILE_LENGTH];
	FILE *f;
	psinfo_t process_info;
	prusage_t process_usage;
#elif defined(HPUX)
#define PROCESS_BATCH 50
	struct pst_status *pstat_procinfo = NULL;
	long procidx = 0;
	long long pagesize;
	int num, i;
#elif defined(AIX)
	struct procentry64 *procs = NULL;
	long long pagesize;
	ssize_t fetched = 0, ncpus;
	pid_t index = 0;
	time_t utime, stime;
	struct timeval now_tval;
	double now_time;
	char *cmndlinebuf = NULL, comm[2*MAXCOMLEN+1];
	/* struct procentry64 curproc_for_getargs; */
#define PROCS_TO_FETCH  250
#elif defined(HAVE_STRUCT_KINFO_PROC2) && defined(KERN_PROC2)
	int mib[6], rc;
	size_t size, argbufsize = ARG_MAX, i;
	struct kinfo_proc2 *kp_stats = NULL, *tmp;
	char *proctitle;
#elif defined(HAVE_STRUCT_KINFO_PROC) && defined(KERN_PROC)
	int mib[6], rc;
	size_t size, nprocs, i;
	struct kinfo_proc *kp_stats = NULL, *tmp;
	char *proctitle;
#  if 0
#   if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD) */
	kvm_t *kvmd;
	char **args, **argsp;
	int argslen = 0;
#   else
	long buflen;
	char *p, *proctitletmp;
#   endif
#  endif
#  ifdef NETBSD2
	size_t lwps;
	struct kinfo_lwp *kl_stats;
#  endif
#endif

#if defined(SOLARIS)
	if( NULL == ( pids_in_proc_dir = scan_proc_dir( PROC_LOCATION ) ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + pids_in_proc_dir->nitems, proc_stats_ptr, sg_process_stats);

	while( pids_in_proc_dir != NULL ) {

		if( pids_in_proc_dir->nitems <= pid_item ) {
			pids_in_proc_dir = free_pids_in_proc_dir( pids_in_proc_dir, false );
			if( pids_in_proc_dir != NULL ) {
				VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + pids_in_proc_dir->nitems, proc_stats_ptr, sg_process_stats);
			}
			pid_item = 0;
			continue;
		}

		snprintf(filename, MAX_FILE_LENGTH, PROC_LOCATION "/" FMT_PID_T "/psinfo", pids_in_proc_dir->items[pid_item]);
		if( ( f = fopen(filename, "r") ) == NULL ) {
			/* Open failed.. Process since vanished, or the path was too long.
			 * Ah well, move onwards to the next one */
			++pid_item;
			continue;
		}

		fread(&process_info, sizeof(psinfo_t), 1, f);
		fclose(f);

		proc_stats_ptr[proc_items].pid = process_info.pr_pid;
		proc_stats_ptr[proc_items].parent = process_info.pr_ppid;
		proc_stats_ptr[proc_items].pgid = process_info.pr_pgid;
		proc_stats_ptr[proc_items].uid = process_info.pr_uid;
		proc_stats_ptr[proc_items].euid = process_info.pr_euid;
		proc_stats_ptr[proc_items].gid = process_info.pr_gid;
		proc_stats_ptr[proc_items].egid = process_info.pr_egid;
		proc_stats_ptr[proc_items].proc_size = (process_info.pr_size) * 1024;
		proc_stats_ptr[proc_items].proc_resident = (process_info.pr_rssize) * 1024;
		proc_stats_ptr[proc_items].start_time = process_info.pr_start.tv_sec;
		proc_stats_ptr[proc_items].time_spent = process_info.pr_time.tv_sec;
		proc_stats_ptr[proc_items].cpu_percent = (process_info.pr_pctcpu * 100.0) / 0x8000;
		proc_stats_ptr[proc_items].nice = (int)process_info.pr_lwp.pr_nice - 20;
		if( ( SG_ERROR_NONE != sg_update_string(&proc_stats_ptr[proc_items].process_name, process_info.pr_fname) ) ||
		    ( SG_ERROR_NONE != sg_update_string(&proc_stats_ptr[proc_items].proctitle, process_info.pr_psargs) ) ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
		}

		switch (process_info.pr_lwp.pr_state) {
		case 1:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
			break;
		case 2:
		case 5:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
			break;
		case 3:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_ZOMBIE;
			break;
		case 4:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
			break;
		}

		snprintf(filename, MAX_FILE_LENGTH, PROC_LOCATION "/" FMT_PID_T "/usage", pids_in_proc_dir->items[pid_item]);
		if( ( f = fopen(filename, "r") ) != NULL ) {

			fread(&process_usage, sizeof(process_usage), 1, f);
			fclose(f);

			proc_stats_ptr[proc_items].context_switches = process_usage.pr_vctx + process_usage.pr_ictx;
			proc_stats_ptr[proc_items].voluntary_context_switches = process_usage.pr_vctx;
			proc_stats_ptr[proc_items].involuntary_context_switches = process_usage.pr_ictx;
		}

		proc_stats_ptr[proc_items].systime = now;
		++pid_item;
		++proc_items;
	}
#elif defined(LINUX)

	if( ( f = fopen(PROC_LOCATION "/uptime", "r") ) == NULL ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_OPEN, PROC_LOCATION "/uptime");
	}
	if( ( fscanf(f, FMT_TIME_T " %*d", &uptime) ) != 1 ) {
		fclose(f);
		RETURN_WITH_SET_ERROR("process", SG_ERROR_PARSE, NULL);
	}
	fclose(f);


	if( NULL == ( pids_in_proc_dir = scan_proc_dir( PROC_LOCATION ) ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

	tickspersec = sysconf (_SC_CLK_TCK);

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP fclose(f);

	VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + pids_in_proc_dir->nitems, proc_stats_ptr, sg_process_stats);

	while( pids_in_proc_dir != NULL ) {

		if( pids_in_proc_dir->nitems <= pid_item ) {
			pids_in_proc_dir = free_pids_in_proc_dir( pids_in_proc_dir, false );
			if( pids_in_proc_dir != NULL ) {
				VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + pids_in_proc_dir->nitems, proc_stats_ptr, sg_process_stats);
			}
			pid_item = 0;
			continue;
		}

		snprintf(filename, MAX_FILE_LENGTH, PROC_LOCATION "/" FMT_PID_T "/stat", pids_in_proc_dir->items[pid_item]);
		if( ( f = fopen(filename, "r") ) == NULL ) {
			/* Open failed.. Process since vanished, or the path was too long.
			 * Ah well, move onwards to the next one */
			++pid_item;
			continue;
		}

		VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + 1, proc_stats_ptr, sg_process_stats);

		fscanf(f, FMT_PID_T "%4096s %c " FMT_PID_T " " FMT_PID_T " %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %d %*d %*d %llu %llu %llu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d\n",
			   &proc_stats_ptr[proc_items].pid, read_buf, &s, &proc_stats_ptr[proc_items].parent,
			   &proc_stats_ptr[proc_items].pgid, &utime, &stime, &proc_stats_ptr[proc_items].nice,
			   &starttime, &proc_stats_ptr[proc_items].proc_size, &proc_stats_ptr[proc_items].proc_resident);
		/* +3 because man page says "Resident  Set Size: number of pages the process has in real memory, minus 3 for administrative purposes." */
		proc_stats_ptr[proc_items].proc_resident = (proc_stats_ptr[proc_items].proc_resident + 3) * sg_get_sys_page_size();
		switch (s) {
		case 'S':
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
			break;
		case 'R':
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
			break;
		case 'Z':
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_ZOMBIE;
			break;
		case 'T':
		case 'D':
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
			break;
		}

		/* pa_name[0] should = '(' */
		read_ptr = strchr(&read_buf[1], ')');
		if( read_ptr != NULL ) {
			*read_ptr = '\0';
		}

		if( SG_ERROR_NONE != sg_update_string( &proc_stats_ptr[proc_items].process_name, &read_buf[1] )) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
		}

		/* cpu */
		proc_stats_ptr[proc_items].cpu_percent = (100.0 * (utime + stime)) / ((uptime * 100.0) - starttime);
		if (tickspersec < 0) {
			proc_stats_ptr[proc_items].time_spent = 0;
			proc_stats_ptr[proc_items].start_time = 0;
		}
		else {
			proc_stats_ptr[proc_items].time_spent = (utime + stime) / tickspersec;
			proc_stats_ptr[proc_items].start_time = (now - uptime) + (starttime / tickspersec);
		}

		fclose(f);

		/* uid / gid */
		snprintf(filename, MAX_FILE_LENGTH, PROC_LOCATION "/" FMT_PID_T "/status", pids_in_proc_dir->items[pid_item]);
		if ((f=fopen(filename, "r")) == NULL) {
			/* Open failed.. Process since vanished, or the path was too long.
			 * Ah well, move onwards to the next one */
			++pid_item;
			continue;
		}

		/* XXX is it sure that "Uid:" is always found before "Gid:"? */
		if( sg_f_read_line(f, read_buf, sizeof(read_buf), "Uid:") == NULL ) {
			fclose(f);
			++pid_item;
			continue;
		}
		sscanf(read_buf, "Uid:\t" FMT_UID_T "\t" FMT_UID_T "\t%*d\t%*d\n",
		       &proc_stats_ptr[proc_items].uid, &proc_stats_ptr[proc_items].euid);

		if( sg_f_read_line(f, read_buf, sizeof(read_buf), "Gid:") == NULL ) {
			fclose(f);
			++pid_item;
			continue;
		}
		sscanf(read_buf, "Gid:\t" FMT_GID_T "\t" FMT_GID_T "\t%*d\t%*d\n",
		       &proc_stats_ptr[proc_items].gid, &proc_stats_ptr[proc_items].egid);

		if( sg_f_read_line(f, read_buf, sizeof(read_buf), "voluntary_ctxt_switches:") != NULL )
			sscanf(read_buf, "voluntary_ctxt_switches:\t%llu", &proc_stats_ptr[proc_items].voluntary_context_switches);
		else
			proc_stats_ptr[proc_items].voluntary_context_switches = 0;

		if( sg_f_read_line(f, read_buf, sizeof(read_buf), "nonvoluntary_ctxt_switches:") != NULL )
			sscanf(read_buf, "nonvoluntary_ctxt_switches:\t%llu", &proc_stats_ptr[proc_items].involuntary_context_switches);
		else
			proc_stats_ptr[proc_items].involuntary_context_switches = 0;

		proc_stats_ptr[proc_items].context_switches = proc_stats_ptr[proc_items].voluntary_context_switches
							    + proc_stats_ptr[proc_items].involuntary_context_switches;

		fclose(f);

		/* proctitle */
		snprintf(filename, MAX_FILE_LENGTH, PROC_LOCATION "/" FMT_PID_T "/cmdline", pids_in_proc_dir->items[pid_item]);

		if( ( fd = open(filename, O_RDONLY) ) == -1 ) {
			/* Open failed.. Process since vanished, or the path was too long.
			 * Ah well, move onwards to the next one */
			++pid_item;
			continue;
		}

		len = 0;
		do {
			rc = read(fd, read_buf, sizeof(read_buf));
			if (rc > 0) {
				read_ptr = sg_realloc( proc_stats_ptr[proc_items].proctitle, len + rc + 1 );
				if( NULL == read_ptr ) {
					free(proc_stats_ptr[proc_items].proctitle);
					proc_stats_ptr[proc_items].proctitle = NULL;
					close(fd);
					RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
				}
				proc_stats_ptr[proc_items].proctitle = read_ptr;
				memcpy( proc_stats_ptr[proc_items].proctitle + len, read_buf, rc );
				len += rc;
			}
		} while (rc > 0);
		close(fd);

		if (rc == -1) {
			/* Read failed; move on. */
			free(proc_stats_ptr[proc_items].proctitle);
			proc_stats_ptr[proc_items].proctitle = NULL;
			++pid_item;
			continue;
		}

		if( proc_stats_ptr[proc_items].proctitle ) { /* no title, no finish 0 byte */
			proc_stats_ptr[proc_items].proctitle[len] = '\0';
			adjust_procname_cmndline( proc_stats_ptr[proc_items].proctitle, len );
		}
		else {
			if( -1 == asprintf( &proc_stats_ptr[proc_items].proctitle, "[%s]", proc_stats_ptr[proc_items].process_name ) ) {
				VECTOR_UPDATE_ERROR_CLEANUP;
				RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_ASPRINTF, NULL);
			}
		}

		proc_stats_ptr[proc_items].systime = now;
		++pid_item;
		++proc_items;
	}
#elif defined(HAVE_STRUCT_KINFO_PROC2) && defined(KERN_PROC2)
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC2;
#if defined(KERN_PROC_KTHREAD)
	mib[2] = KERN_PROC_KTHREAD;
#else
	mib[2] = KERN_PROC_ALL;
#endif
	mib[3] = 0;
	mib[4] = sizeof(*kp_stats);

	if( NULL == (proctitle = sg_malloc( ARG_MAX * sizeof(*proctitle) ) ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

again:
	size = 0;
	mib[5] = 0;
	if( -1 == ( rc = sysctl(mib, 6, NULL, &size, NULL, 0) ) ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "CTL_KERN.KERN_PROC2.KERN_PROC_ALL");
	}
	mib[5] = size / sizeof(*kp_stats);
	if( 0 == ( tmp = sg_realloc( kp_stats, size ) ) ) {
		free(kp_stats);
		free(proctitle);
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

	kp_stats = tmp;
	if( -1 == (rc = sysctl(mib, 6, kp_stats, &size, NULL, (size_t)0 ) ) ) {
		if( errno == ENOMEM ) {
			goto again;
		}
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "CTL_KERN.KERN_PROC2.KERN_PROC_ALL");
	}
	proc_items = size / sizeof(*kp_stats);

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(kp_stats); free(proctitle);
	VECTOR_UPDATE(proc_stats_vector_ptr, proc_items, proc_stats_ptr, sg_process_stats);
	for( i = 0; i < proc_items; ++i ) {

		if( SG_ERROR_NONE != sg_update_string(&proc_stats_ptr[i].process_name, kp_stats[i].p_comm ) ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
		}

		proc_stats_ptr[i].pid = kp_stats[i].p_pid;
		proc_stats_ptr[i].parent = kp_stats[i].p_ppid;
		proc_stats_ptr[i].pgid = kp_stats[i].p__pgid;
		proc_stats_ptr[i].sessid = kp_stats[i].p_sid;

		proc_stats_ptr[i].uid = kp_stats[i].p_uid;
		proc_stats_ptr[i].euid = kp_stats[i].p_ruid;
		proc_stats_ptr[i].gid = kp_stats[i].p_gid;
		proc_stats_ptr[i].egid = kp_stats[i].p_rgid;

		proc_stats_ptr[i].voluntary_context_switches = kp_stats[i].p_uru_nvcsw;
		proc_stats_ptr[i].involuntary_context_switches = kp_stats[i].p_uru_nivcsw;
		proc_stats_ptr[i].context_switches = proc_stats_ptr[i].voluntary_context_switches + proc_stats_ptr[i].involuntary_context_switches;
		proc_stats_ptr[i].proc_size = ((unsigned long long)kp_stats[i].p_vm_dsize) + kp_stats[i].p_vm_ssize;
		proc_stats_ptr[i].proc_size *= sg_get_sys_page_size();
		proc_stats_ptr[i].proc_resident = ((unsigned long long)kp_stats[i].p_vm_rssize) * sg_get_sys_page_size();
		proc_stats_ptr[i].start_time = kp_stats[i].p_ustart_sec;
		proc_stats_ptr[i].time_spent = ((unsigned long long)kp_stats[i].p_uutime_sec) + kp_stats[i].p_ustime_sec;
		proc_stats_ptr[i].cpu_percent = ((double)kp_stats[i].p_cpticks / FSCALE) * 100.0; /* XXX CTL_KERN.KERN_FSCALE */
		proc_stats_ptr[i].nice = kp_stats[i].p_nice;
		switch( kp_stats[i].p_stat )
		{
#if defined(LSIDL)
		case LSIDL:
#elif defined(SIDL)
		case SIDL:
#endif
#if defined(LSSLEEP)
		case LSSLEEP:
#elif defined(SSLEEP)
		case SSLEEP:
#endif
#if defined(LSRUN)
		case LSRUN: /* on RUN queue - maybe soon be chosen for LSONPROC */
#elif defined(SRUN)
		case SRUN: /* on RUN queue - maybe soon be chosen for SONPROC */
#endif
#if defined(LSSUSPENDED)
		case LSSUSPENDED:
#endif
			proc_stats_ptr[i].state = SG_PROCESS_STATE_SLEEPING;
			break;

#if defined(LSDEAD)
		case LSDEAD:
#elif defined(SDEAD)
		case SDEAD:
#endif
#if defined(LSZOMB)
		case LSZOMB:
#elif defined(SZOMB)
		case SZOMB:
#endif
			proc_stats_ptr[i].state = SG_PROCESS_STATE_ZOMBIE;
			break;

#if defined(LSSTOP)
		case LSSTOP:
#elif defined(SSTOP)
		case SSTOP:
#endif
			proc_stats_ptr[i].state = SG_PROCESS_STATE_STOPPED;
			break;

#if defined(LSONPROC)
		case LSONPROC:
#elif defined(SONPROC)
		case SONPROC:
#endif
			proc_stats_ptr[i].state = SG_PROCESS_STATE_RUNNING;
			break;

		default:
			proc_stats_ptr[i].state = SG_PROCESS_STATE_UNKNOWN;
			break;
		}
#if defined(KERN_PROC_ARGS) && defined(KERN_PROC_ARGV)
		if( 0 != kp_stats[i].p_pid ) {
			char *p;

			mib[0] = CTL_KERN;
			mib[1] = KERN_PROC_ARGS;
			mib[2] = kp_stats[i].p_pid;
			mib[3] = KERN_PROC_ARGV;
			*proctitle = '\0';
argv_again:
			size = argbufsize * sizeof(*proctitle);
			rc = sysctl(mib, 4, proctitle, &size, NULL, 0);
			if( -1 == rc ) {
				if( 0 == kp_stats[i].p_ppid && errno == EINVAL ) {
					goto print_kernel_proctitle;
				}
				else if( errno == ENOMEM ) {
					p = sg_realloc( proctitle, size );
					if( NULL == p ) {
						VECTOR_UPDATE_ERROR_CLEANUP;
						RETURN_FROM_PREVIOUS_ERROR("process", sg_get_error() );
					}
					argbufsize = size / sizeof(*proctitle);
					proctitle = p;
					goto argv_again;
				}
				else {
					long failing_pid = kp_stats[i].p_pid;
					VECTOR_UPDATE_ERROR_CLEANUP;
					RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "CTL_KERN.KERN_PROC_ARGS.KERN_PROC_ARGV for pid=%ld", failing_pid);
				}
			}

			if( size > 1 && ( ( p = adjust_procname_cmndline( proctitle, size - 1 ) ) != NULL ) && 0 != strlen(p) ) {
				if( SG_ERROR_NONE != sg_update_string( &proc_stats_ptr[i].proctitle, p ) ) {
					VECTOR_UPDATE_ERROR_CLEANUP;
					RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
				}
			}
			else {
				goto print_kernel_proctitle;
			}

		}
		else {
print_kernel_proctitle:
			if( -1 == asprintf( &proc_stats_ptr[i].proctitle, "[%s]", proc_stats_ptr[i].process_name ) ) {
				long failing_pid = kp_stats[i].p_pid;
				VECTOR_UPDATE_ERROR_CLEANUP;
				RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_ASPRINTF, "pid=%ld", failing_pid);
			}
		}
#endif
		proc_stats_ptr[i].systime = now;
	}

	free(kp_stats);
	free(proctitle);

#elif defined(HAVE_STRUCT_KINFO_PROC) && defined(KERN_PROC)
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
#ifdef KERN_PROC_INC_THREAD
	mib[2] = KERN_PROC_PROC;
	mib[3] = 0;
	i = 4;
#else
	mib[2] = KERN_PROC_ALL;
	i = 3;
#endif

	if( NULL == (proctitle = sg_malloc( ARG_MAX * sizeof(*proctitle) ) ) ) {
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

again:
	size = 0;
	if( -1 == ( rc = sysctl(mib, (unsigned)i, NULL, &size, NULL, 0) ) ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "CTL_KERN.KERN_PROC.KERN_PROC_ALL");
	}
	if( 0 == ( tmp = sg_realloc( kp_stats, size ) ) ) {
		free(kp_stats);
		free(proctitle);
		RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
	}

	kp_stats = tmp;
	if( -1 == (rc = sysctl(mib, (unsigned)i, kp_stats, &size, NULL, (size_t)0) ) ) {
		if( errno == ENOMEM ) {
			goto again;
		}
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "CTL_KERN.KERN_PROC.KERN_PROC_ALL");
	}

	proc_items = 0;
	nprocs = size / sizeof(*kp_stats);

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(kp_stats); free(proctitle);
	VECTOR_UPDATE(proc_stats_vector_ptr, nprocs, proc_stats_ptr, sg_process_stats);
	for( i = 0; i < nprocs; ++i ) {
		const char *name;

# if defined(HAVE_KINFO_PROC_KI_STAT)
		if( kp_stats[i].ki_stat == 0 )
# elif defined(HAVE_KINFO_PROC_KP_PID)
		if( kp_stats[i].kp_stat == 0 )
# elif defined(HAVE_KINFO_PROC_KP_PROC)
		if( kp_stats[i].kp_proc.p_stat == 0 )
# elif defined(HAVE_KINFO_PROC_P_STAT)
		if( kp_stats[i].p_stat == 0 )
# else
		if(0)
# endif
		{
			/* FreeBSD 5 deliberately overallocates the array that
			 * the sysctl returns, so we'll get a few junk
			 * processes on the end that we have to ignore. (Search
			 * for "overestimate by 5 procs" in
			 * src/sys/kern/kern_proc.c for more details.) */
			continue;
		}

# if defined(HAVE_KINFO_PROC_KI_STAT)
		name = kp_stats[proc_items].ki_comm;
# elif defined(HAVE_KINFO_PROC_KP_PID)
		name = kp_stats[proc_items].kp_comm;
# elif defined(HAVE_KINFO_PROC_KP_THREAD)
		name = kp_stats[proc_items].kp_thread.td_comm;
# elif defined(HAVE_KINFO_PROC_KP_PROC)
		name = kp_stats[proc_items].kp_proc.p_comm;
# elif defined(HAVE_KINFO_PROC_P_STAT)
		name = kp_stats[proc_items].p_comm;
# else
		name = "";
# endif
		if( SG_ERROR_NONE != sg_update_string( &proc_stats_ptr[proc_items].process_name, name ) ) {
			VECTOR_UPDATE_ERROR_CLEANUP
			RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
		}

# if defined(HAVE_KINFO_PROC_KI_STAT)
		proc_stats_ptr[proc_items].pid = kp_stats[i].ki_pid;
		proc_stats_ptr[proc_items].parent = kp_stats[i].ki_ppid;
		proc_stats_ptr[proc_items].pgid = kp_stats[i].ki_pgid;
		proc_stats_ptr[proc_items].sessid = kp_stats[i].ki_sid;

		proc_stats_ptr[proc_items].uid = kp_stats[i].ki_ruid;
		proc_stats_ptr[proc_items].euid = kp_stats[i].ki_uid;
		proc_stats_ptr[proc_items].gid = kp_stats[i].ki_rgid;
		proc_stats_ptr[proc_items].egid = kp_stats[i].ki_svgid;

# elif defined(HAVE_KINFO_PROC_KP_EPROC)
		proc_stats_ptr[proc_items].pid = kp_stats[i].kp_proc.p_pid;
		proc_stats_ptr[proc_items].parent = kp_stats[i].kp_eproc.e_ppid;
		proc_stats_ptr[proc_items].pgid = kp_stats[i].kp_eproc.e_pgid;
		proc_stats_ptr[proc_items].sessid = 0;
#  if defined(HAVE_KINFO_PROC_KP_EPROC_E_UCRED_CR_RUID)
		proc_stats_ptr[proc_items].uid = kp_stats[i].kp_eproc.e_ucred.cr_ruid;
		proc_stats_ptr[proc_items].euid = kp_stats[i].kp_eproc.e_ucred.cr_svuid;
		proc_stats_ptr[proc_items].gid = kp_stats[i].kp_eproc.e_ucred.cr_rgid;
		proc_stats_ptr[proc_items].egid = kp_stats[i].kp_eproc.e_ucred.cr_svgid;
#  else
		proc_stats_ptr[proc_items].uid = kp_stats[i].kp_eproc.e_pcred.p_ruid;
		proc_stats_ptr[proc_items].euid = kp_stats[i].kp_eproc.e_pcred.p_svuid;
		proc_stats_ptr[proc_items].gid = kp_stats[i].kp_eproc.e_pcred.p_rgid;
		proc_stats_ptr[proc_items].egid = kp_stats[i].kp_eproc.e_pcred.p_svgid;

		proc_stats_ptr[proc_items].cpu_percent =
			((double)kp_stats[i].kp_proc.p_pctcpu / FSCALE) * 100.0;
#  endif
# elif defined(HAVE_KINFO_PROC_KP_PID)
		proc_stats_ptr[proc_items].pid = kp_stats[i].kp_pid;
		proc_stats_ptr[proc_items].parent = kp_stats[i].kp_ppid == ((pid_t)-1) ? 0 : kp_stats[i].kp_ppid;
		proc_stats_ptr[proc_items].pgid = kp_stats[i].kp_pgid;
		proc_stats_ptr[proc_items].sessid = kp_stats[i].kp_sid;

		proc_stats_ptr[proc_items].uid = kp_stats[i].kp_ruid;
		proc_stats_ptr[proc_items].euid = kp_stats[i].kp_uid;
		proc_stats_ptr[proc_items].gid = kp_stats[i].kp_rgid;
		proc_stats_ptr[proc_items].egid = kp_stats[i].kp_svgid;
#elif defined(HAVE_KINFO_PROC_P_PID)
		proc_stats_ptr[proc_items].pid = kp_stats[i].p_pid;
		proc_stats_ptr[proc_items].parent = kp_stats[i].p_ppid == ((pid_t)-1) ? 0 : kp_stats[i].p_ppid;
		proc_stats_ptr[proc_items].pgid = kp_stats[i].p__pgid;
		proc_stats_ptr[proc_items].sessid = kp_stats[i].p_sid;

		proc_stats_ptr[proc_items].uid = kp_stats[i].p_ruid;
		proc_stats_ptr[proc_items].euid = kp_stats[i].p_uid;
		proc_stats_ptr[proc_items].gid = kp_stats[i].p_rgid;
		proc_stats_ptr[proc_items].egid = kp_stats[i].p_svgid;
# else
		proc_stats_ptr[proc_items].pid = kp_stats[i].kp_proc.p_pid;
		proc_stats_ptr[proc_items].parent = kp_stats[i].kp_eproc.e_ppid;
		proc_stats_ptr[proc_items].pgid = kp_stats[i].kp_eproc.e_pgid;
		proc_stats_ptr[proc_items].sessid = 0;

		proc_stats_ptr[proc_items].time_spent =
			kp_stats[i].kp_proc.p_runtime / 1000000;
		proc_stats_ptr[proc_items].cpu_percent =
			((double)kp_stats[i].kp_proc.p_pctcpu / FSCALE) * 100.0;
# endif


		proc_stats_ptr[proc_items].context_switches = 0;
		proc_stats_ptr[proc_items].voluntary_context_switches = 0;
		proc_stats_ptr[proc_items].involuntary_context_switches = 0;
		proc_stats_ptr[proc_items].start_time = 0;

# if defined(HAVE_KINFO_PROC_KI_STAT)
		/* This is in microseconds */
		proc_stats_ptr[proc_items].time_spent = kp_stats[i].ki_runtime / 1000000;
		proc_stats_ptr[proc_items].cpu_percent = 0; /* ki_pctcpu */
		proc_stats_ptr[proc_items].nice = kp_stats[i].ki_nice;
# elif defined(HAVE_KINFO_PROC_KP_PID)
		proc_stats_ptr[proc_items].time_spent = kp_stats[i].kp_ru.ru_utime.tv_sec;
		proc_stats_ptr[proc_items].time_spent += kp_stats[i].kp_ru.ru_stime.tv_sec;
		proc_stats_ptr[proc_items].cpu_percent = kp_stats[i].kp_lwp.kl_pctcpu;
		proc_stats_ptr[proc_items].time_spent = kp_stats[i].kp_start.tv_sec;
		proc_stats_ptr[proc_items].nice = kp_stats[i].kp_nice;
# elif defined(HAVE_KINFO_PROC_KP_THREAD)
		proc_stats_ptr[proc_items].time_spent =
			( kp_stats[i].kp_thread.td_uticks +
			kp_stats[i].kp_thread.td_sticks +
			kp_stats[i].kp_thread.td_iticks ) / 1000000;
# else
		/* p_cpticks?, p_[usi]ticks? */
#if 0
		/* This is in microseconds */
		proc_stats_ptr[proc_items].time_spent =
			kp_stats[i].kp_proc.p_runtime / 1000000;
#endif
# endif

# if defined(HAVE_KINFO_PROC_KI_STAT)
		proc_stats_ptr[proc_items].proc_size = kp_stats[i].ki_size;
		/* This is in pages */
		proc_stats_ptr[proc_items].proc_resident = ((unsigned long long)kp_stats[i].ki_rssize) * getpagesize();
# elif defined(HAVE_KINFO_PROC_KP_PID)
		proc_stats_ptr[proc_items].proc_size = kp_stats[i].kp_vm_map_size;
		proc_stats_ptr[proc_items].proc_resident = ((unsigned long long)kp_stats[i].kp_vm_rssize) * getpagesize();
# elif defined(HAVE_KINFO_PROC_P_VM_MAP_SIZE)
		proc_stats_ptr[proc_items].proc_size = kp_stats[i].p_vm_map_size;
		proc_stats_ptr[proc_items].proc_resident = ((unsigned long long)kp_stats[i].p_vm_rssize) * getpagesize();
# else
#if 0
		/* This is in microseconds */
		proc_stats_ptr[proc_items].proc_size = kp_stats[i].kp_eproc.e_vm.vm_map.size;
		/* This is in pages */
		proc_stats_ptr[proc_items].proc_resident = kp_stats[i].kp_eproc.e_vm.vm_rssize * getpagesize();
#endif
		proc_stats_ptr[proc_items].nice = kp_stats[i].kp_proc.p_nice;
# endif

# if defined(HAVE_KINFO_PROC_KI_STAT)
		switch (kp_stats[i].ki_stat)
# elif defined(HAVE_KINFO_PROC_KP_PID)
		switch (kp_stats[i].kp_stat)
# elif defined(HAVE_KINFO_PROC_P_STAT)
		switch (kp_stats[i].p_stat)
# else
		switch (kp_stats[i].kp_proc.p_stat)
# endif
		{
# ifdef SRUN
		case SRUN:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
			break;
# endif
# if defined(HAVE_KINFO_PROC_KP_PID)
		case SACTIVE:
			switch(kp_stats[proc_items].kp_lwp.kl_stat)
			{
			case LSRUN:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
				break;

			case LSSTOP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
				break;

			case LSSLEEP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
				break;
			}
			break;
# endif
		case SIDL:
# ifdef SSLEEP
		case SSLEEP:
# endif
# ifdef SWAIT
		case SWAIT: /* FreeBSD 5 */
# endif
# ifdef SLOCK
		case SLOCK: /* FreeBSD 5 */
# endif
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
			break;
		case SSTOP:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
			break;
		case SZOMB:
# ifdef SDEAD
		case SDEAD: /* OpenBSD & NetBSD */
# endif
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_ZOMBIE;
			break;
		default:
			proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_UNKNOWN;
			break;
		}


# if defined(KERN_PROC_ARGS) || defined(KERN_PROCARGS2)
		if( 0 != proc_stats_ptr[proc_items].pid ) {
			char *p;

			size = ARG_MAX * sizeof(*proctitle);
			*proctitle = '\0';
			mib[0] = CTL_KERN;
#  if defined(KERN_PROC_ARGS)
			mib[2] = KERN_PROC;
			mib[2] = KERN_PROC_ARGS;
			mib[3] = ((int)proc_stats_ptr[i].pid);
			rc = 4;
			p = "CTL_KERN.KERN_PROC.KERN_PROC_ARGS";
#  elif defined(KERN_PROCARGS2)
			mib[1] = KERN_PROCARGS2;
			mib[2] = ((int)proc_stats_ptr[i].pid);
			rc = 3;
			p = "CTL_KERN.KERN_PROCARGS2";
#  endif
			if( -1 == ( rc = sysctl(mib, rc, proctitle, &size, NULL, 0) ) ) {
				long failing_pid = (long)proc_stats_ptr[i].pid;
#  if defined(KERN_PROCARGS2)
				if( EINVAL == errno )
					goto print_kernel_proctitle;
#  endif
				RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCTL, "%s for pid=%ld", p, failing_pid);
			}

			if( size > 1 ) {
				adjust_procname_cmndline( proctitle, size );
				if( SG_ERROR_NONE != sg_update_string( &proc_stats_ptr[proc_items].proctitle, proctitle ) ) {
					VECTOR_UPDATE_ERROR_CLEANUP;
					RETURN_FROM_PREVIOUS_ERROR( "process", sg_get_error() );
				}
			}
			else {
				goto print_kernel_proctitle;
			}
		}
		else {
print_kernel_proctitle:
			if( -1 == asprintf( &proc_stats_ptr[proc_items].proctitle, "[%s]", proc_stats_ptr[proc_items].process_name ) ) {
				VECTOR_UPDATE_ERROR_CLEANUP;
				RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_ASPRINTF, NULL);
			}
		}
# endif

		proc_stats_ptr[proc_items].systime = now;
		++proc_items;
	}

	free(kp_stats);
	free(proctitle);
#elif defined(HPUX)
#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(pstat_procinfo);
	if ((pagesize = sysconf(_SC_PAGESIZE)) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

	pstat_procinfo = malloc( PROCESS_BATCH * sizeof(pstat_procinfo[0]) );
	if( NULL == pstat_procinfo ) {
		RETURN_WITH_SET_ERROR( "process", SG_ERROR_MALLOC, "sg_get_process_stats" );
	}

	for(;;) {
		memset( pstat_procinfo, 0, PROCESS_BATCH * sizeof(pstat_procinfo[0]) );
		num = pstat_getproc(pstat_procinfo, sizeof(pstat_procinfo[0]),
				    PROCESS_BATCH, procidx);
		if (num == -1) {
			free(pstat_procinfo);
			RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_PSTAT, "pstat_getproc");
		}
		else if (num == 0) {
			break;
		}

		VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + num, proc_stats_ptr, sg_process_stats);

		for (i = 0; i < num; ++i) {
			struct pst_status *pi = &pstat_procinfo[i];
			sg_error rc;

			proc_stats_ptr[proc_items].pid = pi->pst_pid;
			proc_stats_ptr[proc_items].parent = pi->pst_ppid;
			proc_stats_ptr[proc_items].pgid = pi->pst_pgrp;
			proc_stats_ptr[proc_items].uid = pi->pst_uid;
			proc_stats_ptr[proc_items].euid = pi->pst_euid;
			proc_stats_ptr[proc_items].gid = pi->pst_gid;
			proc_stats_ptr[proc_items].egid = pi->pst_egid;
			proc_stats_ptr[proc_items].context_switches = pi->pst_nvcsw + pi->pst_nivcsw;
			proc_stats_ptr[proc_items].voluntary_context_switches = pi->pst_nvcsw;
			proc_stats_ptr[proc_items].involuntary_context_switches = pi->pst_nivcsw;
			proc_stats_ptr[proc_items].proc_size = (pi->pst_dsize + pi->pst_tsize + pi->pst_ssize) * pagesize;
			proc_stats_ptr[proc_items].proc_resident = pi->pst_rssize * pagesize;
			proc_stats_ptr[proc_items].start_time = pi->pst_start;
			proc_stats_ptr[proc_items].time_spent = pi->pst_time;
			proc_stats_ptr[proc_items].cpu_percent = (pi->pst_pctcpu * 100.0) / 0x8000;
			proc_stats_ptr[proc_items].nice = pi->pst_nice;

			if ( ( SG_ERROR_NONE != ( rc = sg_update_string(&proc_stats_ptr[proc_items].process_name, pi->pst_ucomm ) ) )
			  || ( SG_ERROR_NONE != ( rc = sg_update_string(&proc_stats_ptr[proc_items].proctitle, pi->pst_cmd ) ) ) ) {
				free(pstat_procinfo);
				RETURN_FROM_PREVIOUS_ERROR( "process", rc );
			}

			switch (pi->pst_stat) {
			case PS_SLEEP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
				break;
			case PS_RUN:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
				break;
			case PS_STOP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
				break;
			case PS_ZOMBIE:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_ZOMBIE;
				break;
			case PS_IDLE:
			case PS_OTHER:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_UNKNOWN;
				break;
			}

			proc_stats_ptr[proc_items].systime = now;

			++proc_items;
		}
		procidx = pstat_procinfo[num - 1].pst_idx + 1;
	}
	free(pstat_procinfo);
#elif defined(AIX)
#define	TVALU_TO_SEC(x)	((x).tv_sec + ((double)((x).tv_usec) / 1000000.0))
#define	TVALN_TO_SEC(x)	((x).tv_sec + ((double)((x).tv_usec) / 1000000000.0))
	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	if( -1 == ncpus )
		ncpus = 1; /* sysconf error - assume 1 */

	if ((pagesize = sysconf(_SC_PAGESIZE)) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_SYSCONF, "_SC_PAGESIZE");
	}

	procs = /* (struct procentry64 *) */ malloc(sizeof(*procs) * PROCS_TO_FETCH);
	if(NULL == procs) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_MALLOC, "sg_get_process_stats");
	}

	if( NULL == ( cmndlinebuf = sg_malloc(ARG_MAX + 1) ) ) {
		free(procs);
		RETURN_WITH_SET_ERROR_WITH_ERRNO("process", SG_ERROR_MALLOC, "sg_get_process_stats");
	}
#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP free(procs); free(cmndlinebuf);

	gettimeofday(&now_tval, 0);
	now_time = TVALU_TO_SEC(now_tval);

	/* keep on grabbing chunks of processes until getprocs returns a smaller
	   block than we asked for */
	do {
		char *cmndline = NULL;
		int i;
		memset( procs, 0, sizeof(*procs) * PROCS_TO_FETCH );
		fetched = getprocs64(procs, sizeof(*procs), NULL, 0, &index, PROCS_TO_FETCH);
		VECTOR_UPDATE(proc_stats_vector_ptr, proc_items + fetched, proc_stats_ptr, sg_process_stats);
		for( i = 0; i < fetched; ++i ) {
			struct procentry64 *pi = procs+i;
			int zombie = 0;
			sg_error rc;

			zombie = 0;

			/* set a descriptive name for the process state */
			switch( pi->pi_state ) {
			case SSLEEP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_SLEEPING;
				break;
			case SRUN:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
				break;
			case SZOMB:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_ZOMBIE;
				zombie = 1;
				break;
			case SSTOP:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_STOPPED;
				break;
			case SACTIVE:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_RUNNING;
				break;
			case SIDL:
			default:
				proc_stats_ptr[proc_items].state = SG_PROCESS_STATE_UNKNOWN;
				break;
			}

			if( zombie ) {
				utime = pi->pi_utime;
				stime = pi->pi_stime;
			}
			else {
				utime = TVALN_TO_SEC(pi->pi_ru.ru_utime);
				stime = TVALN_TO_SEC(pi->pi_ru.ru_stime);
			}

			proc_stats_ptr[proc_items].pid = pi->pi_pid;
			proc_stats_ptr[proc_items].parent = pi->pi_ppid;
			proc_stats_ptr[proc_items].pgid = pi->pi_pgrp;
			proc_stats_ptr[proc_items].uid = pi->pi_cred.crx_ruid;
			proc_stats_ptr[proc_items].euid = pi->pi_cred.crx_uid;
			proc_stats_ptr[proc_items].gid = pi->pi_cred.crx_rgid;
			proc_stats_ptr[proc_items].egid = pi->pi_cred.crx_gid;
			proc_stats_ptr[proc_items].context_switches = pi->pi_ru.ru_nvcsw + pi->pi_ru.ru_nivcsw;
			proc_stats_ptr[proc_items].voluntary_context_switches = pi->pi_ru.ru_nvcsw;
			proc_stats_ptr[proc_items].involuntary_context_switches = pi->pi_ru.ru_nivcsw;
			proc_stats_ptr[proc_items].proc_size = pi->pi_dvm * 4096;
			proc_stats_ptr[proc_items].proc_resident = (pi->pi_drss + pi->pi_trss) * 4096; /* XXX might be wrong, see P::PT */
			proc_stats_ptr[proc_items].start_time = pi->pi_start;
			proc_stats_ptr[proc_items].time_spent = utime + stime;
			proc_stats_ptr[proc_items].cpu_percent = (((double)(utime + stime) * 100) / ( now_time - pi->pi_start )) / ncpus;
			proc_stats_ptr[proc_items].nice = pi->pi_nice;
			proc_stats_ptr[proc_items].systime = now;

			/* determine comm & cmndline */
			if( (pi->pi_flags & SKPROC) == SKPROC ) {
				if( pi->pi_pid == 0 ) {
					snprintf(comm, sizeof(comm), "kproc (swapper)");
					cmndline = NULL;
				}
				else {
					snprintf(comm, sizeof(comm), "kproc (%s)", pi->pi_comm);
					cmndline = NULL;
				}
			}
			else {
				snprintf(comm, sizeof(comm), "%s", pi->pi_comm);
				cmndline = cmndlinebuf;
				/*
				curproc_for_getargs.pi_pid = pi->pi_pid;
				if( getargs(&curproc_for_getargs, sizeof(curproc_for_getargs), cmndline, ARG_MAX) < 0 )
					cmndline = NULL;
				else {
					adjust_procname_cmndline( cmndline, 0 );
				}
				*/
				if( getargs(pi, sizeof(*pi), cmndline, ARG_MAX) < 0 )
					cmndline = NULL;
				else {
					adjust_procname_cmndline( cmndline, 0 );
				}
			}

			if( NULL == cmndline )
				rc = sg_update_string( &proc_stats_ptr[proc_items].proctitle, comm );
			else
				rc = sg_update_string( &proc_stats_ptr[proc_items].proctitle, cmndline );

			if( ( SG_ERROR_NONE != rc ) /* see above */
			 || ( SG_ERROR_NONE != ( rc = sg_update_string(&proc_stats_ptr[proc_items].process_name, comm ) ) ) ) {
				VECTOR_UPDATE_ERROR_CLEANUP;
				RETURN_FROM_PREVIOUS_ERROR( "process", rc );
			}

			++proc_items;
		}
	} while( fetched >= PROCS_TO_FETCH );

	free(procs);
	free(cmndlinebuf);
#else
	RETURN_WITH_SET_ERROR("process", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP

	if( proc_stats_vector_ptr && *proc_stats_vector_ptr && ( proc_items != (*proc_stats_vector_ptr)->used_count ) ) {
		VECTOR_UPDATE(proc_stats_vector_ptr, proc_items, proc_stats_ptr, sg_process_stats);
	}

	return SG_ERROR_NONE;
}

MULTI_COMP_ACCESS(sg_get_process_stats,process,process,SG_PROC_STAT_IDX)

int sg_process_compare_name(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return strcmp(a->process_name, b->process_name);
}

int sg_process_compare_pid(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->pid == b->pid) ? 0 : (a->pid < b->pid) ? -1 : 1;
}

int sg_process_compare_uid(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->uid == b->uid) ? 0 : (a->uid < b->uid) ? -1 : 1;
}

int sg_process_compare_gid(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->gid == b->gid) ? 0 : (a->gid < b->gid) ? -1 : 1;
}

int sg_process_compare_size(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->proc_size == b->proc_size) ? 0 : (a->proc_size < b->proc_size) ? -1 : 1;
}

int sg_process_compare_res(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->proc_resident == b->proc_resident) ? 0 : (a->proc_resident < b->proc_resident) ? -1 : 1;
}

int sg_process_compare_cpu(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->cpu_percent == b->cpu_percent) ? 0 : (a->cpu_percent < b->cpu_percent) ? -1 : 1;
}

int sg_process_compare_time(const void *va, const void *vb) {
	const sg_process_stats *a = va, *b = vb;
	return (a->time_spent == b->time_spent) ? 0 : (a->time_spent < b->time_spent) ? -1 : 1;
}

#if 0
#ifdef WIN32
	DWORD aProcesses[1024];
	DWORD cbNeeded;
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return NULL;
	process_stat.total = cbNeeded / sizeof(DWORD);
#endif
#endif

static sg_error
sg_get_process_count_int(sg_process_count *process_count_buf, const sg_vector *process_stats_vector) {
	const sg_process_stats *ps = VECTOR_DATA(process_stats_vector);
	size_t proc_count = process_stats_vector->used_count;

	process_count_buf->total = 0;
	process_count_buf->sleeping = 0;
	process_count_buf->running = 0;
	process_count_buf->zombie = 0;
	process_count_buf->stopped = 0;
	process_count_buf->unknown = 0;

	process_count_buf->total = proc_count;
	process_count_buf->systime = ps->systime;

	/* post increment is intention! */
	while( proc_count-- > 0 ) {
		switch (ps[proc_count].state) {
		case SG_PROCESS_STATE_RUNNING:
			++process_count_buf->running;
			break;
		case SG_PROCESS_STATE_SLEEPING:
			++process_count_buf->sleeping;
			break;
		case SG_PROCESS_STATE_STOPPED:
			++process_count_buf->stopped;
			break;
		case SG_PROCESS_STATE_ZOMBIE:
			++process_count_buf->zombie;
			break;
		case SG_PROCESS_STATE_UNKNOWN:
			++process_count_buf->unknown;
			break;
		default:
			/* currently no mapping for SG_PROCESS_STATE_UNKNOWN in
			 * sg_process_count */
			break;
		}
	}

	return SG_ERROR_NONE;
}

sg_process_count *
sg_get_process_count_of(sg_process_count_source pcs) {

	struct sg_process_glob *process_glob = GLOBAL_GET_TLS(process);
	const sg_vector *process_stats_vector = NULL; /* to avoid wrong warning */
	sg_process_count *process_count;

	TRACE_LOG("process", "entering sg_get_process_count_of");

	if( !process_glob ) {
		/* assuming error comp can't neither */
		ERROR_LOG("process", "sg_get_process_count_of failed - cannot get glob"); \
		return NULL;
	}

	if(!process_glob->process_vectors[SG_PROC_COUNT_IDX])
		process_glob->process_vectors[SG_PROC_COUNT_IDX] = VECTOR_CREATE(sg_process_count, 1);

	if(!process_glob->process_vectors[SG_PROC_COUNT_IDX]){
		SET_ERROR("process", SG_ERROR_MALLOC, "sg_get_process_count_of");
		return NULL;
	}

	process_count = VECTOR_DATA(process_glob->process_vectors[SG_PROC_COUNT_IDX]);

	switch(pcs) {

	case sg_last_process_count:
		process_stats_vector = process_glob->process_vectors[SG_PROC_STAT_IDX];
		if( process_stats_vector )
			break;
		/* else fallthrough */

	case sg_entire_process_count:
		sg_get_process_stats(NULL);
		process_stats_vector = process_glob->process_vectors[SG_PROC_STAT_IDX];
		break;

	default:
		SET_ERROR("process", SG_ERROR_INVALID_ARGUMENT, "sg_get_process_count_of(sg_process_count_source = %d)", (int)pcs);
		break;
	}

	if( !process_stats_vector ) {
		TRACE_LOG_FMT("process", "sg_get_cpu_percents_of failed with %s", sg_str_error(sg_get_error()));
		return NULL;
	}

	if( SG_ERROR_NONE == sg_get_process_count_int(process_count, process_stats_vector) ) {
		TRACE_LOG("process", "sg_get_cpu_percents_of succeded"); \
		return process_count;
	}

	WARN_LOG_FMT("process", "sg_get_cpu_percents_of failed with %s", sg_str_error(sg_get_error()));

	return NULL;
}

sg_process_count *
sg_get_process_count_r(sg_process_stats const * whereof) {

	sg_vector *process_count_result_vector;
	sg_process_count *process_count;
	sg_vector *process_stats_vector = VECTOR_DATA(whereof);

	TRACE_LOG("process", "entering sg_get_process_count_r");

	if( NULL == process_stats_vector ) {
		SET_ERROR("process", SG_ERROR_INVALID_ARGUMENT, "sg_get_process_count_r(whereof = %p)", whereof);
		return NULL;
	}

	process_count_result_vector = VECTOR_CREATE(sg_process_count, 1);
	if( NULL == process_count_result_vector ) {
		ERROR_LOG_FMT("process", "sg_get_process_count_r failed with %s", sg_str_error(sg_get_error()));
		return NULL;
	}

	process_count = VECTOR_DATA(process_count_result_vector);
	if( SG_ERROR_NONE == sg_get_process_count_int(process_count, process_stats_vector) ) {
		TRACE_LOG("process", "sg_get_process_count_r succeded");
		return process_count;
	}

	WARN_LOG_FMT("process", "sg_get_process_count_r failed with %s", sg_str_error(sg_get_error()));
	sg_vector_free(process_count_result_vector);

	return NULL;
}
