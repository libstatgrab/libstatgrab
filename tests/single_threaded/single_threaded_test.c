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

#define NLOOPS 1

struct opt_def opt_def[] = {
#define OPT_HLP 	0
    { 'h', opt_flag, { 0 } },	/* help */
#define OPT_LIST	1
    { 'l', opt_flag, { 0 } },	/* list */
#define OPT_RUN 	2
    { 'r', opt_str, { 0 } },	/* run */
#define OPT_NLOOPS	3
    { 'n', opt_unsigned, { NLOOPS } }, /* num-loops */
    { 0, opt_flag, { 0 } }
};

void
help(char *prgname) {
	printf( "%s -h|-l|-r <test list> [options]\n"
		"\t-h\tshow help\n"
		"\t-l\tlist available test functions\n"
		"\t-r\trun specified list of test calls (comma separated list\n"
		"\t\tof test functions from the list showed by -l)\n"
		"\t-n\tnumber of loops to run (must be greater or equal to 1)\n", prgname );
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
		size_t entries = 0, i;
		struct statgrab_testfuncs *sg_testfuncs = get_testable_functions(&entries);

		for( i = 0; i < entries; ++i ) {
			printf( "%s\n", sg_testfuncs[i].fn_name );
		}

		return 0;
	}
	else if( opt_def[OPT_RUN].optarg.str ) {
		size_t *test_routines = NULL;
		size_t entries = funcnames_to_indices(opt_def[OPT_RUN].optarg.str, &test_routines);

		if( 0 == entries ) {
			die( ESRCH, "no functions to test" );
			return 255;
		}

		while( opt_def[OPT_NLOOPS].optarg.u-- > 0 ) {
			size_t func_rel_idx;

			for( func_rel_idx = 0; func_rel_idx < entries; ++func_rel_idx ) {
				run_func( test_routines[func_rel_idx] );
			}
		}
	}
	else {
		help(argv[0]);
		return 1;
	}

	return 0;
}
