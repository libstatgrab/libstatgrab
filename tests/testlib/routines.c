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

#define SG_TEST_FUNC(name) { #name, (statgrab_multi_fn)(&name), 0, 0 }

static struct statgrab_testfuncs statgrab_tests[] =
{
	SG_TEST_FUNC(sg_get_host_info),
	SG_TEST_FUNC(sg_get_cpu_stats),
	SG_TEST_FUNC(sg_get_mem_stats),
	SG_TEST_FUNC(sg_get_load_stats),
	SG_TEST_FUNC(sg_get_user_stats),
	SG_TEST_FUNC(sg_get_swap_stats),
	SG_TEST_FUNC(sg_get_fs_stats),
	SG_TEST_FUNC(sg_get_disk_io_stats),
	SG_TEST_FUNC(sg_get_network_io_stats),
	SG_TEST_FUNC(sg_get_page_stats),
	SG_TEST_FUNC(sg_get_process_stats)
};

struct statgrab_testfuncs *
get_testable_functions(size_t *entries) {
	if(entries)
		*entries = lengthof(statgrab_tests);

	return statgrab_tests;
}

static size_t
find_func(const char *func_name, size_t namelen) {
	size_t idx;

	for( idx = 0; idx < lengthof(statgrab_tests); ++idx ) {
		if( 0 == strncmp( func_name, statgrab_tests[idx].fn_name, namelen ) ) /* XXX memcmp? */
			break;
	}

	return idx;
}

size_t
funcnames_to_indices(const char *name_list, size_t **indices) {
	size_t i = 0;
	const char *name_start;

	if( ( indices == NULL ) || ( *indices != NULL ) )
		return 0;
	*indices = calloc( lengthof(statgrab_tests), sizeof(indices[0]) );
	if( *indices == NULL )
		return 0;

	for( name_start = name_list; *name_list; ++name_list ) {
		if( ',' == *name_list ) {
			size_t idx = find_func( name_start, name_list - name_start );
			if( idx >= lengthof(statgrab_tests) ) {
				fprintf( stderr, "invalid function name for testing: %s\n", name_start );
				exit(255);
			}
			(*indices)[i++] = idx;
			name_start = name_list + 1;
			DEBUG_LOG_FMT( "testlib", "funcnames_to_indices: found function %s", statgrab_tests[idx].fn_name );
		}
	}

	if( name_start < name_list ) {
		size_t idx = find_func( name_start, name_list - name_start );
		if( idx >= lengthof(statgrab_tests) ) {
			fprintf( stderr, "invalid function name for testing: %s\n", name_start );
			exit(255);
		}
		(*indices)[i++] = idx;
		name_start = name_list + 1;
		DEBUG_LOG_FMT( "testlib", "funcnames_to_indices: found function %s", statgrab_tests[idx].fn_name );
	}

	return i;
}

void
mark_func(size_t func_index) {
	if( func_index >= lengthof(statgrab_tests) ) {
		fprintf( stderr, "mark_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	++statgrab_tests[func_index].needed;
}

void
run_func(size_t func_index) {
	size_t entries;

	if( func_index >= lengthof(statgrab_tests) ) {
		fprintf( stderr, "run_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	INFO_LOG_FMT( "testlib", "Calling %s...", statgrab_tests[func_index].fn_name );
	statgrab_tests[func_index].fn(&entries);
	INFO_LOG_FMT( "testlib", "%s - entries = %lu", statgrab_tests[func_index].fn_name, entries );
}

void
done_func(size_t func_index) {
	if( func_index >= lengthof(statgrab_tests) ) {
		fprintf( stderr, "done_func: index out of range: %lu\n", (unsigned long int)(func_index) );
		exit(1);
	}

	++statgrab_tests[func_index].done;
}
