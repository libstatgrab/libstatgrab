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
#include "testlib.h"

#define SG_EASY_FUNC(stat,full) { #stat, \
                                       #full, (statgrab_stat_fn)(&full), \
				       NULL, NULL, \
				       0, 0, 0 }

#define SG_COMPLEX_FUNC(stat,full,diff) { #stat, \
                                       #full, (statgrab_stat_fn)(&full), \
				       #diff, (statgrab_stat_fn)(&diff), \
				       0, 0, 0 }

static struct statgrab_testfuncs statgrab_test_funcs[] =
{
	SG_EASY_FUNC(host_info, sg_get_host_info),
	SG_COMPLEX_FUNC(cpu_stats, sg_get_cpu_stats, sg_get_cpu_stats_diff), /* XXX cpu_percent */
	SG_EASY_FUNC(mem_stats, sg_get_mem_stats),
	SG_EASY_FUNC(load_stats, sg_get_load_stats),
	SG_EASY_FUNC(user_stats, sg_get_user_stats),
	SG_EASY_FUNC(swap_stats, sg_get_swap_stats),
	SG_COMPLEX_FUNC(fs_stats, sg_get_fs_stats, sg_get_fs_stats_diff),
	SG_COMPLEX_FUNC(disk_io_stats, sg_get_disk_io_stats,sg_get_disk_io_stats_diff),
	SG_EASY_FUNC(network_iface_stats, sg_get_network_iface_stats),
	SG_COMPLEX_FUNC(network_io_stats, sg_get_network_io_stats, sg_get_network_io_stats_diff),
	SG_COMPLEX_FUNC(page_stats, sg_get_page_stats, sg_get_page_stats_diff),
	SG_EASY_FUNC(process_stats, sg_get_process_stats) /* XXX process count */
};

struct statgrab_testfuncs *
get_testable_functions(size_t *entries) {
	if(entries)
		*entries = lengthof(statgrab_test_funcs);

	return statgrab_test_funcs;
}

static size_t
find_full_func(const char *func_name, size_t namelen) {
	size_t idx;

	for( idx = 0; idx < lengthof(statgrab_test_funcs); ++idx ) {
		if( 0 == strncmp( func_name, statgrab_test_funcs[idx].full_name, namelen ) ) /* XXX memcmp? */
			break;
	}

	return idx;
}

static size_t
find_diff_func(const char *func_name, size_t namelen) {
	size_t idx;

	for( idx = 0; idx < lengthof(statgrab_test_funcs); ++idx ) {
		if( NULL == statgrab_test_funcs[idx].diff_name )
			continue;
		if( 0 == strncmp( func_name, statgrab_test_funcs[idx].diff_name, namelen ) ) /* XXX memcmp? */
			break;
	}

	return idx;
}

#define NAME_OF(a,full) full ? (a).full_name : (a).diff_name

size_t
funcnames_to_indices(const char *name_list, size_t **indices, int full) {
	size_t i = 0;
	const char *name_start;

	if( ( indices == NULL ) || ( *indices != NULL ) )
		return 0;
	*indices = calloc( lengthof(statgrab_test_funcs), sizeof(indices[0]) );
	if( *indices == NULL )
		return 0;

	for( name_start = name_list; *name_list; ++name_list ) {
		if( ',' == *name_list ) {
			size_t idx = full
			             ? find_full_func( name_start, name_list - name_start )
			             : find_diff_func( name_start, name_list - name_start );
			if( idx >= lengthof(statgrab_test_funcs) ) {
				fprintf( stderr, "invalid function name for testing: %s\n", name_start );
				exit(255);
			}
			DEBUG_LOG_FMT( "testlib", "funcnames_to_indices: found function %s", NAME_OF(statgrab_test_funcs[i],full) );
			(*indices)[i++] = idx;
			name_start = name_list + 1;
		}
	}

	if( name_start < name_list ) {
		size_t idx = full
		             ? find_full_func( name_start, name_list - name_start )
		             : find_diff_func( name_start, name_list - name_start );
		if( idx >= lengthof(statgrab_test_funcs) ) {
			fprintf( stderr, "invalid function name for testing: %s\n", name_start );
			exit(255);
		}
		DEBUG_LOG_FMT( "testlib", "funcnames_to_indices: found function %s", NAME_OF(statgrab_test_funcs[i],full) );
		(*indices)[i++] = idx;
		name_start = name_list + 1;
	}

	return i;
}

void
print_testable_functions(int full)
{
	size_t i;

	for( i = 0; i < lengthof(statgrab_test_funcs); ++i ) {
		if( !full && NULL == statgrab_test_funcs[i].diff_name )
			continue;
		printf( "%s\n", full ? statgrab_test_funcs[i].full_name : statgrab_test_funcs[i].diff_name );
	}
}

size_t
report_testable_functions(int full)
{
	size_t i, ok = 0;
	for( i = 0; i < lengthof(statgrab_test_funcs); ++i ) {
		if(0 != statgrab_test_funcs[i].needed)
			printf( "%s (%s) - needed: %d, succeeded: %d, done: %d\n",
				statgrab_test_funcs[i].stat_name,
				full ? statgrab_test_funcs[i].full_name : statgrab_test_funcs[i].diff_name,
				statgrab_test_funcs[i].needed,
				statgrab_test_funcs[i].succeeded,
				statgrab_test_funcs[i].done );
		if( statgrab_test_funcs[i].needed == statgrab_test_funcs[i].done )
			++ok;
	}

	return ok;
}

void
mark_func(size_t func_index) {
	if( func_index >= lengthof(statgrab_test_funcs) ) {
		fprintf( stderr, "mark_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	++statgrab_test_funcs[func_index].needed;
}

int
run_func(size_t func_index, int full) {
	size_t entries;
	void *stats;

	if( func_index >= lengthof(statgrab_test_funcs) ) {
		fprintf( stderr, "run_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	INFO_LOG_FMT( "testlib", "Calling %s...", NAME_OF(statgrab_test_funcs[func_index],full) );
	stats = full
	      ? statgrab_test_funcs[func_index].full_fn(&entries)
	      : statgrab_test_funcs[func_index].diff_fn(&entries);
#if !(defined(WITH_LIBLOG4CPLUS) || defined(WITH_FULL_CONSOLE_LOGGER))
	if(!stats) {
		sg_error_details err_det;
		char *errmsg = NULL;
		sg_error errc;

		if( SG_ERROR_NONE != ( errc = sg_get_error_details(&err_det) ) ) {
			fprintf(stderr, "panic: can't get error details (%d, %s)\n", errc, sg_str_error(errc));
		}
		else if( NULL == sg_strperror(&errmsg, &err_det) ) {
			errc = sg_get_error();
			fprintf(stderr, "panic: can't prepare error message (%d, %s)\n", errc, sg_str_error(errc));
		}
		else {
			fprintf( stderr, "%s: %s\n", NAME_OF(statgrab_test_funcs[func_index],full), errmsg );
		}

		free( errmsg );
	}
#endif
	INFO_LOG_FMT( "testlib", "%s - stats = %p, entries = %lu", NAME_OF(statgrab_test_funcs[func_index],full), stats, entries );

	return stats != 0;
}

void
done_func(size_t func_index, int succeeded) {
	if( func_index >= lengthof(statgrab_test_funcs) ) {
		fprintf( stderr, "done_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	++statgrab_test_funcs[func_index].done;
	if(succeeded)
		++statgrab_test_funcs[func_index].succeeded;
}
