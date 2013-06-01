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

#include "tools.h"

struct sg_comp_info {
    const struct sg_comp_init * const init_comp;
    size_t glob_ofs;
};

extern const struct sg_comp_init sg_cpu_init;
extern const struct sg_comp_init sg_disk_init;
extern const struct sg_comp_init sg_error_init;
extern const struct sg_comp_init sg_load_init;
extern const struct sg_comp_init sg_mem_init;
extern const struct sg_comp_init sg_network_init;
extern const struct sg_comp_init sg_os_init;
extern const struct sg_comp_init sg_page_init;
extern const struct sg_comp_init sg_process_init;
extern const struct sg_comp_init sg_swap_init;
extern const struct sg_comp_init sg_user_init;

static struct sg_comp_info comp_info[] = {
	/* error must be the first component to initialize - to hold
	 * the errors for later coming ones */
	{ &sg_error_init, 0 },
	{ &sg_cpu_init, 0 },
	{ &sg_disk_init, 0 },
	{ &sg_load_init, 0 },
	{ &sg_mem_init, 0 },
	{ &sg_network_init, 0 },
	{ &sg_os_init, 0 },
	{ &sg_page_init, 0 },
	{ &sg_process_init, 0 },
	{ &sg_swap_init, 0 },
	{ &sg_user_init, 0 },
};

#define GLOB_MASK MAGIC_EYE('g','l','o','b')

static size_t glob_size = 0;

#ifdef ENABLE_THREADS
static void *sg_alloc_globals(void);
static void sg_destroy_globals(void *);
# if defined(HAVE_PTHREAD)
#  include <pthread.h>
static pthread_key_t glob_key;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
# elif defined(_WIN32)
#  include <windows.h>
static DWORD glob_tls_idx = TLS_OUT_OF_INDEXES;
# else
#  error unsupport thread library
# endif

static void
sg_destroy_main_globals(void)
{
#if defined(HAVE_PTHREAD)
	char *glob_buf;
	while( NULL != ( glob_buf = pthread_getspecific(glob_key) ) )
		sg_destroy_globals(glob_buf);
#endif
}

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
struct sg_named_mutex {
	const char *name;
	pthread_mutex_t mutex;
};

static struct sg_named_mutex glob_lock = { "statgrab", PTHREAD_MUTEX_INITIALIZER };
static struct sg_named_mutex *required_locks = NULL;
static size_t num_required_locks = 0;
static unsigned initialized = 0;

static int
cmp_named_locks( const void *va, const void *vb ) {

	const struct sg_named_mutex *a = va, *b = vb;

	assert( a->name );
	assert( b->name );

	return strcmp( a->name, b->name );
}

sg_error
sg_lock_mutex(const char *mutex_name) {

	struct sg_named_mutex *found;
	int rc;

	found = bsearch( &mutex_name, required_locks, num_required_locks, sizeof(required_locks[0]), cmp_named_locks );
	if( NULL == found ) {
		ERROR_LOG_FMT("globals", "Can't find mutex '%s' to be locked ...", mutex_name);
		RETURN_WITH_SET_ERROR( "globals", SG_ERROR_INVALID_ARGUMENT, mutex_name );
	}

	TRACE_LOG_FMT("globals", "going to lock mutex '%s' for thread %p", mutex_name, (void *)(pthread_self()));
	rc = pthread_mutex_lock( &found->mutex );
	if( 0 != rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE("globals", SG_ERROR_MUTEX_LOCK, rc, "error %d locking mutex '%s' for thread %p", rc, mutex_name, (void *)(pthread_self()));
	}

	TRACE_LOG_FMT("globals", "mutex '%s' locked for thread %p", mutex_name, (void *)(pthread_self()));

	return SG_ERROR_NONE;
}

sg_error
sg_unlock_mutex(const char *mutex_name) {

	struct sg_named_mutex *found;
	int rc;

	found = bsearch( &mutex_name, required_locks, num_required_locks, sizeof(required_locks[0]), cmp_named_locks );
	if( NULL == found ) {
		ERROR_LOG_FMT("globals", "Can't find mutex '%s' to be unlocked ...", mutex_name);
		RETURN_WITH_SET_ERROR( "globals", SG_ERROR_INVALID_ARGUMENT, mutex_name );
	}

	TRACE_LOG_FMT("globals", "going to unlock mutex '%s' in thread %p", mutex_name, (void *)(pthread_self()));
	rc = pthread_mutex_unlock( &found->mutex );
	if( 0 != rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE("globals", SG_ERROR_MUTEX_LOCK, rc, "error %d unlocking mutex '%s' in thread %p", rc, mutex_name, (void *)(pthread_self()));
	}

	TRACE_LOG_FMT("globals", "mutex '%s' unlocked in thread %p", mutex_name, (void *)(pthread_self()));

	return SG_ERROR_NONE;
}

sg_error
sg_global_lock(void)
{
	int rc;

	TRACE_LOG_FMT("globals", "going to lock mutex 'statgrab' for thread %p", (void *)(pthread_self()));
	rc = pthread_mutex_lock( &glob_lock.mutex );
	if( 0 != rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE("globals", SG_ERROR_MUTEX_LOCK, rc, "error %d locking mutex '%s' in thread %p", rc, "statgrab", (void *)(pthread_self()));
	}
	TRACE_LOG_FMT("globals", "mutex 'statgrab' locked for thread %p", (void *)(pthread_self()));

	return SG_ERROR_NONE;
}

sg_error
sg_global_unlock(void)
{
	int rc;

	TRACE_LOG_FMT("globals", "going to unlock mutex 'statgrab' for thread %p", (void *)(pthread_self()));
	rc = pthread_mutex_unlock( &glob_lock.mutex );
	if( 0 != rc ) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO_CODE("globals", SG_ERROR_MUTEX_LOCK, rc, "error %d unlocking mutex '%s' in thread %p", rc, "statgrab", (void *)(pthread_self()));
	}
	TRACE_LOG_FMT("globals", "mutex 'statgrab' unlocked for thread %p", (void *)(pthread_self()));

	return SG_ERROR_NONE;
}

static void
sg_first_init(void)
{
	pthread_mutexattr_t attr;
	int rc;

	rc = pthread_key_create(&glob_key, sg_destroy_globals);
	if( 0 != rc )
		abort(); // can't store error state without TLS
	rc = pthread_mutexattr_init(&attr);
	if( 0 != rc )
		abort();
	rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	if( 0 != rc )
		abort();
	rc = pthread_mutex_init(&glob_lock.mutex, NULL);
	if( 0 != rc )
		abort(); // can't store error state without TLS
}
#endif

static sg_error
sg_init_thread_local(void) {
# if defined(HAVE_PTHREAD)
	if( 0 != pthread_once(&once_control, sg_first_init) )
		abort(); // can't store error state without TLS
# elif defined(_WIN32)
	if( ( TLS_OUT_OF_INDEXES == glob_tls_idx ) && ( ( glob_tls_idx = TlsAlloc()) == TLS_OUT_OF_INDEXES ) )
		abort(); // can't store error state without TLS
# endif

	atexit((void (*)(void))sg_destroy_main_globals);
	return SG_ERROR_NONE;
}
#else /* !ENABLE_THREADS */
static char *glob_buf = NULL;
#endif /* ?ENABLE_THREADS */

sg_error
sg_comp_init(int ignore_init_errors) {
	unsigned i;
	sg_error errc = SG_ERROR_NONE;

	TRACE_LOG("globals", "Entering sg_comp_init() ...");

#ifdef ENABLE_THREADS
	errc = sg_init_thread_local();
	if(SG_ERROR_NONE != errc)
		return errc;

	sg_global_lock();
	if( 0 != initialized++ )
		return sg_global_unlock();
#endif

	TRACE_LOG("globals", "Doing sg_comp_init() ...");
	glob_size = 0;

	for( i = 0; i < lengthof(comp_info); ++i ) {
		comp_info[i].glob_ofs = glob_size;
		glob_size += comp_info[i].init_comp->static_buf_size;
	}

	TRACE_LOG_FMT("globals", "need %lu bytes tls storage for %lu components", glob_size, i);

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
	required_locks = sg_malloc( sizeof(required_locks[0]) );
	if( NULL == required_locks ) {
		RETURN_WITH_SET_ERROR("globals", SG_ERROR_MALLOC, "sg_comp_init");
	}
	required_locks[0] = glob_lock;
	num_required_locks = 1;
#else
	num_required_locks = 0;
	required_locks = 0;
#endif

	for( i = 0; i < lengthof(comp_info); ++i ) {
#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
		unsigned mtx_idx, new_mtx_count;
		struct sg_named_mutex *tmp = NULL;
#endif

		if( comp_info[i].init_comp->init_fn ) {
			TRACE_LOG_FMT("globals", "calling init_fn for comp %u", i);
			sg_error comp_errc = comp_info[i].init_comp->init_fn(i + GLOB_MASK);
			if( comp_errc != SG_ERROR_NONE ) {
				if( ignore_init_errors && (NULL != comp_info[i].init_comp->status) ) {
					char *buf = NULL;
					comp_info[i].init_comp->status->init_error = comp_errc;
					ERROR_LOG_FMT("globals", "sg_comp_init: comp[%u].init_fn() fails with '%s'", i, sg_strperror(&buf, NULL));
					free(buf);
					errc = SG_ERROR_INITIALISATION;
				}
				else {
					RETURN_FROM_PREVIOUS_ERROR( "globals", comp_errc );
				}
			}
		}

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
		for( mtx_idx = 0, new_mtx_count = 0; comp_info[i].init_comp->required_locks[mtx_idx] != NULL; ++mtx_idx ) {
			if( NULL != required_locks ) {
				/* looking whether the required mutex is already known by it's name */
				tmp = bsearch( &comp_info[i].init_comp->required_locks[mtx_idx], required_locks,
					       num_required_locks, sizeof(required_locks[0]),
					       cmp_named_locks );
				if( NULL != tmp )
					continue; /* skip already known ... */
			}
			++new_mtx_count;
		}

		if( 0 == new_mtx_count )
			continue; /* to next comp */

		tmp = sg_realloc(required_locks, sizeof(required_locks[0]) * (num_required_locks + new_mtx_count) );
		if( NULL == tmp ) {
			RETURN_WITH_SET_ERROR("globals", SG_ERROR_MALLOC, "sg_comp_init");
		}
		required_locks = tmp;

		mtx_idx = 0;
		while( comp_info[i].init_comp->required_locks[mtx_idx] != NULL ) {
			/* looking whether the required mutex is already known by it's name (probably double requirement)) */
			tmp = bsearch( &comp_info[i].init_comp->required_locks[mtx_idx], required_locks,
				       num_required_locks, sizeof(required_locks[0]), cmp_named_locks );
			if( NULL != tmp ) {
				++mtx_idx;
				continue;
			}
			required_locks[num_required_locks++].name = comp_info[i].init_comp->required_locks[mtx_idx++];

			/* sort to allow bsearch for next name in loop */
			qsort( required_locks, num_required_locks, sizeof(required_locks[0]), cmp_named_locks );
		}
#endif
	}

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
	if( 0 != num_required_locks )
	{
		pthread_mutexattr_t attr;
		int rc;

		rc = pthread_mutexattr_init(&attr);
		if( 0 != rc ) {
			PANIC_LOG_FMT( "globals", "sg_comp_init: pthread_mutexattr_init() fails with %d", rc );
			return SG_ERROR_INITIALISATION;
		}
		rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		if( 0 != rc ) {
			PANIC_LOG_FMT( "globals", "sg_comp_init: pthread_mutexattr_settype() fails with %d", rc );
			return SG_ERROR_INITIALISATION;
		}

		for( i = 0; i < num_required_locks; ++i ) {
			if( required_locks[i].name != glob_lock.name )
				pthread_mutex_init( &required_locks[i].mutex, &attr );
		}

		pthread_mutexattr_destroy(&attr);
	}
#endif

	sg_global_unlock();

	TRACE_LOG_FMT("globals", "%lu components initialized", i);

	return errc;
}

sg_error
sg_comp_destroy(void) {

	unsigned i;

	sg_global_lock();
	if( 0 != --initialized )
		return sg_global_unlock();

	glob_size = 0;

	for( i = lengthof(comp_info) - 1; i < lengthof(comp_info) /*overflow!*/; --i ) {
		if( NULL != comp_info[i].init_comp->destroy_fn)
			comp_info[i].init_comp->destroy_fn();
	}

#if defined(ENABLE_THREADS) && defined(HAVE_PTHREAD)
	for( i = 0; i < num_required_locks; ++i ) {
		if( glob_lock.name == required_locks[i].name )
			continue;
		pthread_mutex_destroy(&required_locks[i].mutex);
	}
	free( required_locks );
	num_required_locks = 0;
#endif

	return sg_global_unlock();
}

void *
sg_alloc_globals(void) {
#ifdef ENABLE_THREADS
	void *glob_buf = NULL;
# if defined(HAVE_PTHREAD)
	TRACE_LOG_FMT("globals", "allocating globals for thread %p", (void *)(pthread_self()));
# elif defined(WIN32)
	TRACE_LOG_FMT("globals", "allocating globals for thread %u", GetCurrentThreadId() );
# endif
#endif
	if( 0 != glob_size) {
# if defined(HAVE_PTHREAD)
		int rc;
#endif
		glob_buf = malloc(glob_size);
		if(NULL == glob_buf){
			SET_ERROR("globals", SG_ERROR_MALLOC, "sg_alloc_globals: malloc() failed or no components registered");
			return NULL;
		}

		memset( glob_buf, 0, glob_size );
		TRACE_LOG_FMT("globals", "allocated globals zeroed from %p .. %p (%lu bytes)",
			      glob_buf, ((char *)glob_buf) + glob_size, glob_size);

#ifdef ENABLE_THREADS
# if defined(HAVE_PTHREAD)
		if( 0 != ( rc = pthread_setspecific(glob_key, glob_buf) ) ) {
			ERROR_LOG_FMT("globals", "Couldn't set allocated global storage for thread %p: %d",
				      (void *)(pthread_self()), rc );
			free( glob_buf );
			glob_buf = 0;
		}
		else {
			TRACE_LOG_FMT("globals", "globals for thread %p set to %p",
				      (void *)(pthread_self()), glob_buf);
		}
# elif defined(WIN32)
		TlsSetValue(glob_tls_idx, glob_buf);
		// XXX error check!
# endif
#endif
	}

	return glob_buf;
}

void
sg_destroy_globals(void *glob_buf){
#ifdef ENABLE_THREADS
# if defined(HAVE_PTHREAD)
	TRACE_LOG_FMT("globals", "destroying global storage at %p for thread %p", glob_buf, (void *)(pthread_self()));
# elif defined(WIN32)
	glob_buf = TlsGetValue(glob_tls_idx);
	TRACE_LOG_FMT("globals", "destroying global storage at %p for thread %u", glob_buf, GetCurrentThreadId() );
	// (void)ign; /* to avoid unused parameter warning */
# endif
#endif
	if( glob_buf ) {
		size_t i = lengthof(comp_info) - 1, zero_size;
# if defined(HAVE_PTHREAD)
		int rc;
# endif
		zero_size = comp_info[i].glob_ofs + comp_info[i].init_comp->static_buf_size;
		for( i = lengthof(comp_info) - 1; i < lengthof(comp_info) /*overflow!*/; --i ) {
			if(comp_info[i].init_comp->cleanup_fn)
				comp_info[i].init_comp->cleanup_fn(((char *)glob_buf) + comp_info[i].glob_ofs);
		}
		memset( glob_buf, 0, zero_size );
		free( glob_buf );
		glob_buf = 0;
#ifdef ENABLE_THREADS
# if defined(HAVE_PTHREAD)
		if( 0 != ( rc = pthread_setspecific(glob_key, NULL) ) ) {
			ERROR_LOG_FMT("globals", "Couldn't set freed global storage for thread %p: %d", (void *)(pthread_self()), rc );
		}
# elif defined(WIN32)
		// XXX sure?
		TlsSetValue(glob_tls_idx, NULL);
# endif
#endif
	}
}

void *
sg_comp_get_tls(unsigned id) {
#ifdef ENABLE_THREADS
	char *glob_buf;
# if defined(HAVE_PTHREAD)
	glob_buf = pthread_getspecific(glob_key);
# elif defined(WIN32)
	glob_buf = TlsGetValue(glob_tls_idx);
	if((NULL == glob_buf) && (ERROR_SUCCESS != GetLastError())){
		RETURN_WITH_SET_ERROR("globals", SG_ERROR_MEMSTATUS, NULL);
	}
# endif
#endif

	if( NULL == glob_buf )
		glob_buf = sg_alloc_globals();

	if( /* still */ NULL == glob_buf )
		return NULL; /* check sg_error() */

	id -= GLOB_MASK;
	if( id >= lengthof(comp_info) ) {
		SET_ERROR("globals", SG_ERROR_INVALID_ARGUMENT, "sg_comp_get_tls: invalid id (%u)", id);
		return NULL;
	}

	TRACE_LOG_FMT("globals", "get_global_static(%u): %p", id, glob_buf + comp_info[id].glob_ofs);
	return glob_buf + comp_info[id].glob_ofs;
}
