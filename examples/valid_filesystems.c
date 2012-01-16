/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2011 i-scream
 * Copyright (C) 2011 Jens Rehsack
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

/* A very basic example of how to get the list of valid file systems from the
 * system and display them.
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv) {
	size_t nvalid_fs = 0;
	const char **valid_fs;

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

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

