/*
 * libstatgrab
 * https://libstatgrab.org
 * Copyright (C) 2011-2013 i-scream
 * Copyright (C) 2011-2013 Jens Rehsack
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/* A very basic example of how to get the list of valid file systems from the
 * system and display them.
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

#ifdef HAVE_LOG4CPLUS_INITIALIZE
static void *l4cplus_initializer;

static void
cleanup_logging(void)
{
	log4cplus_deinitialize(l4cplus_initializer);
}
#endif

int main(int argc, char **argv) {
	size_t nvalid_fs = 0;
	const char **valid_fs;

#ifdef HAVE_LOG4CPLUS_INITIALIZE
	l4cplus_initializer = log4cplus_initialize();
	atexit((void (*)(void))cleanup_logging);
#endif

	/* Initialise helper - e.g. logging, if any */
	sg_log_init("libstatgrab-examples", "SGEXAMPLES_LOG_PROPERTIES", argc ? argv[0] : NULL);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		sg_die("Error. Failed to drop privileges", 1);
	}

	valid_fs = sg_get_valid_filesystems(&nvalid_fs);
	if( valid_fs )  {
		size_t n = 0;
		printf( "Valid file systems:\n" );
		for( n = 0; n < nvalid_fs; ++n ) {
			printf( "  - %s\n", valid_fs[n] );
		}
	}

	return 0;
}

