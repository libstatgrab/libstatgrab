/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2010-2013 i-scream
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
#include <tools.h>
#include <testlib.h>

static void
prove_libcall(char *libcall, int err_code)
{
	if( err_code != 0 )
		die( err_code, "invoking %s failed", libcall );
}

/* For safe condition variable usage, must use a boolean predicate and  */
/* a mutex with the condition.                                          */
int conditionMet = 0;
size_t test_counter = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
#ifdef NEED_PTHREAD_MUTEX_INITIALIZER_BRACES
pthread_mutex_t mutex = { PTHREAD_MUTEX_INITIALIZER };
#else
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

struct opt_def opt_def[] = {
#define OPT_HLP 	0
    { 'h', opt_flag, { 0 } },	/* help */
#define OPT_LIST	1
    { 'l', opt_flag, { 0 } },	/* list */
#define OPT_RUN 	2
    { 'r', opt_str, { 0 } },	/* run */
#define OPT_NTHREADS	3
    { 'n', opt_unsigned, { -1 } },	/* num-threads */
#define OPT_MTHREADS	4
    { 'm', opt_unsigned, { 1 } },	/* multiple threads */
#define OPT_SEQ 	5
    { 's', opt_bool, { 0 } },	/* sequential test execution */
    { 0, opt_flag, { 0 } }
};

void *
threadfunc(void *parm)
{
	int rc, success;
	size_t func_idx = *((size_t *)parm);

	rc = pthread_mutex_lock(&mutex);
	prove_libcall("pthread_mutex_lock", rc);

	++test_counter;

	while (!conditionMet) {
		TRACE_LOG( "multi_threaded", "Thread blocked" );
		rc = pthread_cond_wait(&cond, &mutex);
		prove_libcall("pthread_cond_wait", rc);
	}

	if( !opt_def[OPT_SEQ].optarg.b ) {
		rc = pthread_mutex_unlock(&mutex);
		prove_libcall("pthread_mutex_unlock", rc);
	}

	success = run_func( func_idx, 0 );

	if( !opt_def[OPT_SEQ].optarg.b ) {
		rc = pthread_mutex_lock(&mutex);
		prove_libcall("pthread_mutex_lock", rc);
	}

	done_func(func_idx, success);

	rc = pthread_mutex_unlock(&mutex);
	prove_libcall("pthread_mutex_unlock", rc);

	pthread_cond_signal(&cond);

	return NULL;
}

void
help(char *prgname) {
	printf( "%s -h|-l|-r <test list> [options]\n"
		"\t-h\tshow help\n"
		"\t-l\tlist available test functions\n"
		"\t-r\trun specified list of test calls (comma separated list\n"
		"\t\tof test functions from the list showed by -l)\n"
		"\t-n\tnumber of threads to use (must be greater or equal to\n"
		"\t\tnumber of test calls)\n"
		"\t-m\tmultiple threads/calls per function\n"
		"\t-s\tsequencial calling of test functions\n", prgname );
}

int
main(int argc, char **argv) {
	log_init( argc, argv );
	sg_init(1);

	if( 0 != get_params( opt_def, argc, argv ) ) {
		help(argv[0]);
		return 1;
	}

	if( opt_def[OPT_HLP].optarg.b ) {
		help(argv[0]);
		return 0;
	}
	else if( opt_def[OPT_LIST].optarg.b ) {
		print_testable_functions(0);
		return 0;
	}
	else if( opt_def[OPT_RUN].optarg.str ) {
		size_t numthreads, i, nfuncs, ok;
		size_t *test_routines = NULL;
		struct statgrab_testfuncs *sg_testfuncs = get_testable_functions(&nfuncs);
		size_t entries = funcnames_to_indices(opt_def[OPT_RUN].optarg.str, &test_routines, 0);
		pthread_t *threadid = NULL;
		int rc, errors = 0;

		if( 0 == entries ) {
			die( ESRCH, "no functions to test" );
			return 255;
		}
		if( -1 != opt_def[OPT_NTHREADS].optarg.s ) {
			numthreads = opt_def[OPT_NTHREADS].optarg.u;
			if( numthreads < entries ) {
				die( ERANGE, "%s %s - to small number for thread count", argv[0], argv[2] );
			}
		}
		else if( opt_def[OPT_MTHREADS].optarg.u > 1 ) {
			numthreads = entries * opt_def[OPT_MTHREADS].optarg.u;
		}
		else {
			numthreads = entries;
		}

		if( NULL == ( threadid = calloc( sizeof(threadid[0]), numthreads ) ) )
			die( ENOMEM, "%s", argv[0] );

		rc = pthread_mutex_lock(&mutex);
		prove_libcall("pthread_mutex_lock", rc);

		TRACE_LOG_FMT( "multi_threaded", "create %zu threads", numthreads );
		for( i = 0; i < numthreads; ++i ) {
			mark_func(test_routines[i % entries]);
			rc = pthread_create( &threadid[i], NULL, threadfunc, &test_routines[i % entries] );
			prove_libcall("pthread_create", rc);
		}

		rc = pthread_mutex_unlock(&mutex);
		prove_libcall("pthread_mutex_unlock", rc);

		while( test_counter < numthreads )
			sched_yield();

		rc = pthread_mutex_lock(&mutex);
		prove_libcall("pthread_mutex_lock", rc);

		/* The condition has occured. Set the flag and wake up any waiting threads */
		conditionMet = 1;
		TRACE_LOG( "multi_threaded", "Wake up all waiting threads..." );
		rc = pthread_cond_broadcast(&cond);
		prove_libcall("pthread_cond_broadcast", rc);

		rc = pthread_mutex_unlock(&mutex);
		prove_libcall("pthread_mutex_unlock", rc);

		TRACE_LOG( "multi_threaded", "Wait for threads and cleanup" );
		do {
			struct timespec ts = { 1, 0 };
			struct timeval tv;

			gettimeofday(&tv, NULL);
			ts.tv_sec += tv.tv_sec;

			rc = pthread_mutex_lock(&mutex);
			prove_libcall("pthread_mutex_lock", rc);

			pthread_cond_timedwait(&cond, &mutex, &ts);
			prove_libcall("pthread_cond_timedwait", rc);

			ok = report_testable_functions(0);

			rc = pthread_mutex_unlock(&mutex);
			prove_libcall("pthread_mutex_unlock", rc);

			if( ok != nfuncs )
				printf( "---------------\n" );
			fflush(stdout);
		} while( ok != nfuncs );

		for (i=0; i<numthreads; ++i) {
			rc = pthread_join(threadid[i], NULL);
			prove_libcall("pthread_join", rc);
		}
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);

		for( i = 0; i < nfuncs; ++i )
			errors += sg_testfuncs[i].needed - sg_testfuncs[i].succeeded;

		TRACE_LOG_FMT( "multi_threaded", "Main completed with test_counter = %zu", test_counter );

		return errors;
	}

	help(argv[0]);
	return 1;
}
