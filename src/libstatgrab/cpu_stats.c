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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id$
 */

#include "tools.h"

#define SG_CPU_NOW_IDX		0
#define SG_CPU_DIFF_IDX		1
#define SG_CPU_PERCENT_IDX	2
#define SG_CPU_IDX_COUNT	3

#if defined(ALLBSD) \
	&& !defined(KERN_CP_TIME) && !defined(KERN_CPTIME) \
	&& !(defined(HAVE_HOST_STATISTICS) || defined(HAVE_HOST_STATISTICS64)) \
	&& !(defined(HAVE_STRUCT_KINFO_CPUTIME) && defined(HAVE_KINFO_GET_SCHED_CPUTIME))
EXTENDED_COMP_SETUP(cpu,SG_CPU_IDX_COUNT,NULL);

static int cp_time_mib[2];

sg_error
sg_cpu_init_comp(unsigned id){
	size_t len = lengthof(cp_time_mib);
	GLOBAL_SET_ID(cpu,id);

	if( -1 == sysctlnametomib( "kern.cp_time", cp_time_mib, &len ) ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_SYSCTLNAMETOMIB, "kern.cp_time");
	}

	return SG_ERROR_NONE;
}

EASY_COMP_DESTROY_FN(cpu)
EASY_COMP_CLEANUP_FN(cpu,SG_CPU_IDX_COUNT)
#else
EASY_COMP_SETUP(cpu,SG_CPU_IDX_COUNT,NULL);
#endif

/* Internal function to fill cpu_stats. No need to resize - we don't have
 * different stats per physical cpu.
 */
static sg_error
sg_get_cpu_stats_int(sg_cpu_stats *cpu_stats_buf) {

#ifdef HPUX
	struct pst_dynamic pstat_dynamic;
	struct pst_vminfo pstat_vminfo;
	int i;
#elif defined(AIX)
	perfstat_cpu_total_t all_cpu_info;
	int rc;
#elif defined(SOLARIS)
	kstat_ctl_t *kc;
	kstat_t *ksp;
	cpu_stat_t cs;
#elif defined(LINUX) || defined(CYGWIN)
	FILE *f;
	int proc_stat_cpu;
	char line[1024];
	int matched = 0;
#elif defined(ALLBSD)
# if defined(HAVE_HOST_STATISTICS) || defined(HAVE_HOST_STATISTICS64)
	host_cpu_load_info_data_t cpustats;
	mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
	mach_port_t self_host_port;
	kern_return_t rc;
#define DEFINED_MIB
	int mib[2];
# elif defined(HAVE_STRUCT_KINFO_CPUTIME) && defined(HAVE_KINFO_GET_SCHED_CPUTIME)
	int rc;
	struct kinfo_cputime cpu_time;
# elif defined(KERN_CP_TIME)
	int mib[2] = { CTL_KERN, KERN_CP_TIME };
	uint64_t cp_time[CPUSTATES];
#define DEFINED_MIB
# elif defined(KERN_CPTIME)
	int mib[2] = { CTL_KERN, KERN_CPTIME };
	long cp_time[CPUSTATES];
#define DEFINED_MIB
# else
	int mib[2] = { cp_time_mib[0], cp_time_mib[1] };
	long cp_time[CPUSTATES];
#define DEFINED_MIB
# endif

	size_t size;
# if defined(VM_UVMEXP2) && defined(HAVE_STRUCT_UVMEXP_SYSCTL)
	struct uvmexp_sysctl uvmexp;
#  ifndef DEFINED_MIB
	int mib[2];
#  endif
# elif defined(VM_UVMEXP) && defined(HAVE_STRUCT_UVMEXP_SWTCH)
	struct uvmexp uvmexp;
#  ifndef DEFINED_MIB
	int mib[2];
#  endif
# elif defined(HAVE_STRUCT_VMMETER) && defined(VM_METER)
	struct vmmeter vmmeter;
#  ifndef DEFINED_MIB
#define DEFINED_MIB
	int mib[2];
#  endif
# endif
#endif

	memset( cpu_stats_buf, 0, sizeof(*cpu_stats_buf) );
#if 0
	cpu_stats_buf->user=0;
	/* Not stored in linux or freebsd */
	cpu_stats_buf->iowait=0;
	cpu_stats_buf->kernel=0;
	cpu_stats_buf->idle=0;
	/* Not stored in linux, freebsd, hpux, aix or windows */
	cpu_stats_buf->swap=0;
	cpu_stats_buf->total=0;
	/* Not stored in aix, solaris or windows */
	cpu_stats_buf->nice=0;
#endif

#ifdef HPUX
	if (pstat_getdynamic(&pstat_dynamic, sizeof(pstat_dynamic), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_PSTAT, "pstat_dynamic");
	}
	if (pstat_getvminfo(&pstat_vminfo, sizeof(pstat_vminfo), 1, 0) == -1) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_PSTAT, "pstat_vminfo");
	}
	cpu_stats_buf->user   = pstat_dynamic.psd_cpu_time[CP_USER];
	cpu_stats_buf->iowait = pstat_dynamic.psd_cpu_time[CP_WAIT];
	cpu_stats_buf->kernel = pstat_dynamic.psd_cpu_time[CP_SSYS] + pstat_dynamic.psd_cpu_time[CP_SYS];
	cpu_stats_buf->idle   = pstat_dynamic.psd_cpu_time[CP_IDLE];
	cpu_stats_buf->nice   = pstat_dynamic.psd_cpu_time[CP_NICE];
	for (i = 0; i < PST_MAX_CPUSTATES; i++) {
		cpu_stats_buf->total += pstat_dynamic.psd_cpu_time[i];
	}

	cpu_stats_buf->context_switches = pstat_vminfo.psv_sswtch;
	cpu_stats_buf->syscalls = pstat_vminfo.psv_ssyscall;
	/* voluntary/involuntary context switches only per process */
	cpu_stats_buf->interrupts = pstat_vminfo.psv_sintr;
	/* cpu_stats_buf->soft_interrupts = unknown: psv_sfaults, psv_strap?; */
#elif defined(AIX)
	bzero(&all_cpu_info, sizeof(all_cpu_info));
	rc = perfstat_cpu_total( NULL, &all_cpu_info, sizeof(all_cpu_info), 1);
	if( -1 == rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_PSTAT, "perfstat_cpu_total");
	}

	cpu_stats_buf->user   = all_cpu_info.user;
	cpu_stats_buf->iowait = all_cpu_info.wait;
	cpu_stats_buf->kernel = all_cpu_info.sys;
	cpu_stats_buf->idle   = all_cpu_info.idle;
	cpu_stats_buf->total  = all_cpu_info.user + all_cpu_info.wait + all_cpu_info.sys + all_cpu_info.idle;

	cpu_stats_buf->context_switches = all_cpu_info.pswitch;
	cpu_stats_buf->syscalls = all_cpu_info.syscall;
	/* voluntary/involuntary context switches only per cpu (summarize) on AIX6+ */
	cpu_stats_buf->interrupts = all_cpu_info.devintrs;
	cpu_stats_buf->soft_interrupts = all_cpu_info.softintrs;

#elif defined(SOLARIS)
	if ((kc = kstat_open()) == NULL) {
		RETURN_WITH_SET_ERROR("cpu", SG_ERROR_KSTAT_OPEN, NULL);
	}
	for (ksp = kc->kc_chain; ksp!=NULL; ksp = ksp->ks_next) {
		if (ksp->ks_type != KSTAT_TYPE_RAW)
			continue;
		if ((strcmp(ksp->ks_module, "cpu_stat")) != 0)
			continue;
		if (kstat_read(kc, ksp, &cs) == -1)
			continue;
		cpu_stats_buf->user += (long long)cs.cpu_sysinfo.cpu[CPU_USER];
		cpu_stats_buf->kernel += (long long)cs.cpu_sysinfo.cpu[CPU_KERNEL];
		cpu_stats_buf->idle += (long long)cs.cpu_sysinfo.cpu[CPU_IDLE];
		cpu_stats_buf->iowait += (long long)cs.cpu_sysinfo.wait[W_IO]+(long long)cs.cpu_sysinfo.wait[W_PIO];
		cpu_stats_buf->swap += (long long)cs.cpu_sysinfo.wait[W_SWAP];

		cpu_stats_buf->context_switches += (long long)cs.cpu_sysinfo.pswitch;
		cpu_stats_buf->syscalls += (long long)cs.cpu_sysinfo.syscall;
		cpu_stats_buf->involuntary_context_switches += (long long)cs.cpu_sysinfo.inv_swtch;
		cpu_stats_buf->interrupts += (long long)cs.cpu_sysinfo.intr;
		/* cpu_stats_buf->soft_interrupts += (long long)cs.cpu_sysinfo.? trap?; */
	}

	cpu_stats_buf->total = cpu_stats_buf->user + cpu_stats_buf->iowait + cpu_stats_buf->kernel + cpu_stats_buf->idle + cpu_stats_buf->swap;
	cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->context_switches - cpu_stats_buf->involuntary_context_switches;

	kstat_close(kc);
#elif defined(LINUX) || defined(CYGWIN)
	if ((f=fopen("/proc/stat", "r" ))==NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_OPEN, "/proc/stat");
	}

	while( ( fgets(line, sizeof(line), f) != NULL ) && (matched < 4) ) {
		if( ( ( strncmp( line, "cpu", strlen("cpu" ) ) == 0 ) && isblank(line[strlen("cpu")]) ) ) {
			/* The very first line should be cpu */
			proc_stat_cpu = sscanf(line, "cpu %llu %llu %llu %llu %llu",
				&cpu_stats_buf->user,
				&cpu_stats_buf->nice,
				&cpu_stats_buf->kernel,
				&cpu_stats_buf->idle,
				&cpu_stats_buf->iowait);

			if (proc_stat_cpu < 4 || proc_stat_cpu > 5) {
				RETURN_WITH_SET_ERROR("cpu", SG_ERROR_PARSE, "cpu");
			}

			++matched;
		}
		else if( ( ( strncmp( line, "intr", strlen("intr" ) ) == 0 ) && isblank(line[strlen("intr")]) ) ) {
			proc_stat_cpu = sscanf(line, "intr %llu", &cpu_stats_buf->interrupts);

			if (proc_stat_cpu != 1) {
				RETURN_WITH_SET_ERROR("cpu", SG_ERROR_PARSE, "intr");
			}

			++matched;
		}
		else if( ( ( strncmp( line, "ctxt", strlen("ctxt" ) ) == 0 ) && isblank(line[strlen("ctxt")]) ) ) {
			proc_stat_cpu = sscanf(line, "ctxt %llu", &cpu_stats_buf->context_switches);

			if (proc_stat_cpu != 1) {
				RETURN_WITH_SET_ERROR("cpu", SG_ERROR_PARSE, "ctxt");
			}

			++matched;
		}
		else if( ( ( strncmp( line, "softirq", strlen("softirq" ) ) == 0 ) && isblank(line[strlen("softirq")]) ) ) {
			proc_stat_cpu = sscanf(line, "softirq %llu", &cpu_stats_buf->soft_interrupts);

			if (proc_stat_cpu != 1) {
				RETURN_WITH_SET_ERROR("cpu", SG_ERROR_PARSE, "softirq");
			}

			++matched;
		}
	}

	fclose(f);

	cpu_stats_buf->total = cpu_stats_buf->user + cpu_stats_buf->nice + cpu_stats_buf->kernel + cpu_stats_buf->idle;
#elif defined(ALLBSD)
# if defined(HAVE_HOST_STATISTICS) || defined(HAVE_HOST_STATISTICS64)
	self_host_port = mach_host_self();
	if( (rc = host_statistics(self_host_port, HOST_CPU_LOAD_INFO, &cpustats, &count)) != KERN_SUCCESS ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE( "cpu", SG_ERROR_MACHCALL, rc, "host_statistics" );
	}

	cpu_stats_buf->user = cpustats.cpu_ticks[CPU_STATE_USER];
	cpu_stats_buf->nice = cpustats.cpu_ticks[CPU_STATE_NICE];
	cpu_stats_buf->kernel = cpustats.cpu_ticks[CPU_STATE_SYSTEM];
	cpu_stats_buf->idle = cpustats.cpu_ticks[CPU_STATE_IDLE];
	cpu_stats_buf->total = cpu_stats_buf->user + cpu_stats_buf->nice + cpu_stats_buf->kernel + cpu_stats_buf->idle;
# elif defined(HAVE_STRUCT_KINFO_CPUTIME) && defined(HAVE_KINFO_GET_SCHED_CPUTIME)
	if( (rc = kinfo_get_sched_cputime( &cpu_time ) ) != 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO( "cpu", SG_ERROR_SYSCTL, "kern.cputime" );
	}

	cpu_stats_buf->user = cpu_time.cp_user;
	cpu_stats_buf->nice = cpu_time.cp_nice;
	cpu_stats_buf->kernel = cpu_time.cp_sys;
	cpu_stats_buf->kernel += cpu_time.cp_intr;
	cpu_stats_buf->idle = cpu_time.cp_idle;
	cpu_stats_buf->total = cpu_stats_buf->user + cpu_stats_buf->nice + cpu_stats_buf->kernel + cpu_stats_buf->idle;
# else
	/* FIXME probably better rewrite and use sysctlgetmibinfo
	 *       or sysctl( {0, 4, ...}, ... ) to know surely returned item size */
	size = sizeof(cp_time);
	if (sysctl(mib, 2, &cp_time, &size, NULL, 0) < 0) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_SYSCTL,
#  if defined(KERN_CPTIME)
						 "CTL_KERN.KERN_CPTIME"
#  elif defined(KERN_CP_TIME)
						 "CTL_KERN.KERN_CP_TIME"
#  else
						 "nametomib(kern.cp_time)"
# endif
		);
	}

	/* XXX clock ticks! */
	cpu_stats_buf->user=cp_time[CP_USER];
	cpu_stats_buf->nice=cp_time[CP_NICE];
	cpu_stats_buf->kernel=cp_time[CP_SYS];
	cpu_stats_buf->idle=cp_time[CP_IDLE];

	cpu_stats_buf->total = 0;
	for( size = 0; size < CPUSTATES; ++size )
		cpu_stats_buf->total += cp_time[size];
# endif
# if defined(VM_UVMEXP2) && defined(HAVE_STRUCT_UVMEXP_SYSCTL) /* read: NetBSD */
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP2;
	size = sizeof(uvmexp);
	if( sysctl( mib, 2, &uvmexp, &size, NULL, 0 ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP2" );
	}
	cpu_stats_buf->context_switches = uvmexp.swtch;
	cpu_stats_buf->syscalls = uvmexp.syscalls;
	cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->involuntary_context_switches = 0;
	cpu_stats_buf->interrupts = uvmexp.intrs;
	cpu_stats_buf->soft_interrupts = uvmexp.softs;
# elif defined(VM_UVMEXP) && defined(HAVE_STRUCT_UVMEXP_SWTCH) /* read: OpenBSD */
	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;
	size = sizeof(uvmexp);
	if( sysctl( mib, 2, &uvmexp, &size, NULL, 0 ) < 0 ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("cpu", SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP" );
	}
	cpu_stats_buf->context_switches = uvmexp.swtch;
	cpu_stats_buf->syscalls = uvmexp.syscalls;
	cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->involuntary_context_switches = 0;
	cpu_stats_buf->interrupts = uvmexp.intrs;
	cpu_stats_buf->soft_interrupts = uvmexp.softs;
# elif defined(HAVE_STRUCT_VMMETER) && defined(VM_METER)
	mib[0] = CTL_VM;
	mib[1] = VM_METER;
	size = sizeof(vmmeter);
	if( sysctl( mib, 2, &vmmeter, &size, NULL, 0 ) == 0 ) {
		cpu_stats_buf->context_switches = vmmeter.v_swtch;
		cpu_stats_buf->syscalls = vmmeter.v_syscall;
		cpu_stats_buf->syscalls += vmmeter.v_trap;
		cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->involuntary_context_switches = 0;
		cpu_stats_buf->interrupts = vmmeter.v_intr;
		cpu_stats_buf->soft_interrupts = vmmeter.v_soft;
	}
	else {
		cpu_stats_buf->context_switches = 0;
		cpu_stats_buf->syscalls = 0;
		cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->involuntary_context_switches = 0;
		cpu_stats_buf->interrupts = 0;
		cpu_stats_buf->soft_interrupts = 0;
	}
# else
	cpu_stats_buf->context_switches = 0;
	cpu_stats_buf->syscalls = 0;
	cpu_stats_buf->voluntary_context_switches = cpu_stats_buf->involuntary_context_switches = 0;
	cpu_stats_buf->interrupts = 0;
	cpu_stats_buf->soft_interrupts = 0;
# endif

#else
	cpu_stats_buf->systime = 0;
	RETURN_WITH_SET_ERROR("cpu", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	cpu_stats_buf->systime=time(NULL);

	return SG_ERROR_NONE;
}

VECTOR_INIT_INFO_EMPTY_INIT(sg_cpu_stats);

EASY_COMP_ACCESS(sg_get_cpu_stats,cpu,cpu,SG_CPU_NOW_IDX)

static unsigned long long
counter_diff(unsigned long long new, unsigned long long old){
# if (defined(VM_UVMEXP2) && defined(HAVE_STRUCT_UVMEXP_SYSCTL)) || /* read: NetBSD */ \
     (defined(VM_UVMEXP) && defined(HAVE_STRUCT_UVMEXP_SWTCH)) /* read: OpenBSD */
	/* 64-bit quantities, so plain subtraction works. */
	return new - old;
# elif defined(HAVE_STRUCT_VMMETER) && defined(VM_METER)
	/* probably 32-bit quantities, so we must explicitly deal with wraparound. */
	if( new >= old ) {
		return new - old;
	}
	else {
		unsigned long long retval = INT_MAX;
		++retval;
		retval += new;
		retval -= old;
		return retval;
	}
#else
	/* 64-bit quantities, so plain subtraction works. */
	return new - old;
#endif
}

static sg_error
sg_get_cpu_stats_diff_int(sg_cpu_stats *tgt, const sg_cpu_stats *now, const sg_cpu_stats *last){

	if( NULL == tgt ) {
		RETURN_WITH_SET_ERROR("cpu", SG_ERROR_INVALID_ARGUMENT, "sg_get_cpu_stats_diff_int(tgt)");
	}

	if( now ) {
		*tgt = *now;
		if( last ) {
			tgt->user -= last->user;
			tgt->kernel -= last->kernel;
			tgt->idle -= last->idle;
			tgt->iowait -= last->iowait;
			tgt->swap -= last->swap;
			tgt->nice -= last->nice;
			tgt->total -= last->total;
			tgt->context_switches = counter_diff( tgt->context_switches, last->context_switches );
			tgt->voluntary_context_switches = counter_diff( tgt->voluntary_context_switches, last->voluntary_context_switches );
			tgt->involuntary_context_switches = counter_diff( tgt->involuntary_context_switches, last->involuntary_context_switches );
			tgt->syscalls = counter_diff( tgt->syscalls, last->syscalls );
			tgt->interrupts = counter_diff( tgt->interrupts, last->interrupts );
			tgt->soft_interrupts = counter_diff( tgt->soft_interrupts, last->soft_interrupts );
			tgt->systime -= last->systime;
		}
	}
	else {
		memset( tgt, 0, sizeof(*tgt) );
	}

	return SG_ERROR_NONE;
}

EASY_COMP_DIFF(sg_get_cpu_stats_diff,sg_get_cpu_stats,cpu,cpu,SG_CPU_DIFF_IDX,SG_CPU_NOW_IDX)

VECTOR_INIT_INFO_EMPTY_INIT(sg_cpu_percents);

static sg_error
sg_get_cpu_percents_int(sg_cpu_percents *cpu_percent_buf, const sg_cpu_stats *cpu_stats_buf){

	cpu_percent_buf->user =  ((double)cpu_stats_buf->user) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->kernel =  ((double)cpu_stats_buf->kernel) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->idle = ((double)cpu_stats_buf->idle) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->iowait = ((double)cpu_stats_buf->iowait) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->swap = ((double)cpu_stats_buf->swap) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->nice = ((double)cpu_stats_buf->nice) / ((double)cpu_stats_buf->total) * 100;
	cpu_percent_buf->time_taken = cpu_stats_buf->systime;

	return SG_ERROR_NONE;
}
#if 0
/* this is nonsense - according to the documentation at
 * http://support.microsoft.com/kb/262938 is it possible for other
 * applications to read the same global counter and reset the the
 * measure points.
 *
 * better take a look to
 * - http://technet.microsoft.com/en-us/library/cc737309%28WS.10%29.aspx
 * - SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION at
 *   http://msdn.microsoft.com/en-us/library/ms724509%28VS.85%29.aspx
 * It's unlikely that this API will be removed before full replacement
 * API is available.
 */
#ifdef WIN32
	double result;

	if(read_counter_double(SG_WIN32_PROC_USER, &result)) {
		sg_set_error(SG_ERROR_PDHREAD, PDH_USER);
		return NULL;
	}
	cpu_usage->user = (float)result;
	if(read_counter_double(SG_WIN32_PROC_PRIV, &result)) {
		sg_set_error(SG_ERROR_PDHREAD, PDH_PRIV);
		return NULL;
	}
	cpu_usage->kernel = (float)result;
	if(read_counter_double(SG_WIN32_PROC_IDLE, &result)) {
		sg_set_error(SG_ERROR_PDHREAD, PDH_IDLE);
		return NULL;
	}
	/* win2000 does not have an idle counter, but does have %activity
	 * so convert it to idle */
	cpu_usage->idle = 100 - (float)result;
	if(read_counter_double(SG_WIN32_PROC_INT, &result)) {
		sg_set_error(SG_ERROR_PDHREAD, PDH_INTER);
		return NULL;
	}
	cpu_usage->iowait = (float)result;

	return SG_ERROR_NONE;
#endif
#endif

sg_cpu_percents *
sg_get_cpu_percents_of(sg_cpu_percent_source cps){

	struct sg_cpu_glob *cpu_glob = GLOBAL_GET_TLS(cpu);
	sg_vector *cpu_stats_vector = NULL; /* to avoid wrong warning */
	sg_cpu_stats *cs_ptr = NULL;
	sg_cpu_percents *cpu_usage;

	TRACE_LOG("cpu", "entering sg_get_cpu_percents_of");

	if( !cpu_glob ) {
		/* assuming error comp can't neither */
		ERROR_LOG("cpu", "sg_get_cpu_percents_of failed - cannot get glob");
		return NULL;
	}

	if( !cpu_glob->cpu_vectors[SG_CPU_PERCENT_IDX] )
		cpu_glob->cpu_vectors[SG_CPU_PERCENT_IDX] = VECTOR_CREATE(sg_cpu_percents, 1);

	if( !cpu_glob->cpu_vectors[SG_CPU_PERCENT_IDX] ) {
		sg_set_error_fmt(SG_ERROR_MALLOC, "sg_get_cpu_percents_of");
		ERROR_LOG_FMT("cpu", "sg_get_cpu_percents_of failed with %s", sg_str_error(SG_ERROR_MALLOC));
		return NULL;
	}

	cpu_usage = VECTOR_DATA(cpu_glob->cpu_vectors[SG_CPU_PERCENT_IDX]);

	switch(cps) {

	case sg_entire_cpu_percent:
		cpu_stats_vector = cpu_glob->cpu_vectors[SG_CPU_NOW_IDX];
		if( !cpu_stats_vector )
			cs_ptr = sg_get_cpu_stats();
		break;

	case sg_last_diff_cpu_percent:
		cpu_stats_vector = cpu_glob->cpu_vectors[SG_CPU_DIFF_IDX];
		if( cpu_stats_vector )
			break;
		/* else fallthrough */

	case sg_new_diff_cpu_percent:
		cs_ptr = sg_get_cpu_stats_diff();
		break;

	default:
		SET_ERROR("cpu", SG_ERROR_INVALID_ARGUMENT, "sg_get_cpu_percents_of(cps = %d)", (int)cps);
		break;
	}

	if( !cpu_stats_vector && !cs_ptr ) {
		ERROR_LOG_FMT("cpu", "sg_get_cpu_percents_of failed with %s", sg_str_error(sg_get_error()));
		return NULL; /* error should been set by called subroutine */
	}

	if( cpu_stats_vector && !cs_ptr )
		cs_ptr = VECTOR_DATA(cpu_stats_vector);
	if( SG_ERROR_NONE == sg_get_cpu_percents_int(cpu_usage, cs_ptr) ) {
		TRACE_LOG("cpu", "sg_get_cpu_percents_of succeded"); \
		return cpu_usage;
	}

	WARN_LOG_FMT("cpu", "sg_get_cpu_percents_of failed with %s", sg_str_error(sg_get_error()));

	return NULL;
}

sg_cpu_percents *
sg_get_cpu_percents_r(sg_cpu_stats const * whereof){

	sg_vector *cpu_percents_result_vector;
	sg_cpu_percents *cpu_usage;

	TRACE_LOG("cpu", "entering sg_get_cpu_percents_r");

	if( NULL == whereof ) {
		SET_ERROR("cpu", SG_ERROR_INVALID_ARGUMENT, "sg_get_cpu_percents_r(whereof = %p)", whereof);
		return NULL;
	}

	cpu_percents_result_vector = VECTOR_CREATE(sg_cpu_percents, 1);
	if( NULL == cpu_percents_result_vector ) {
		ERROR_LOG_FMT("cpu", "sg_get_cpu_percents_r failed with %s", sg_str_error(sg_get_error()));
		return NULL;
	}

	cpu_usage = VECTOR_DATA(cpu_percents_result_vector);
	if( SG_ERROR_NONE == sg_get_cpu_percents_int(cpu_usage, whereof) ) {
		TRACE_LOG("cpu", "sg_get_cpu_percents_r succeded");
		return cpu_usage;
	}

	WARN_LOG_FMT("cpu", "sg_get_cpu_percents_r failed with %s", sg_str_error(sg_get_error()));

	sg_vector_free(cpu_percents_result_vector);

	return NULL;
}
