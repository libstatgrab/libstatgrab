/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2010,2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

void
log_init(int argc, char **argv) {
#ifdef WITH_LIBLOG4CPLUS
	char proppath[PATH_MAX];
	char *p;
	int rc = 0;

	if(argc >= 1) {
		p = proppath + sg_strlcpy( proppath, argv[0], PATH_MAX );
		strncpy( p, ".properties", PATH_MAX - ( p - proppath ) );
		if( ( rc = access( proppath, R_OK ) ) < 1 ) {
			char dnbuf[PATH_MAX];
			strncpy( dnbuf, argv[0], PATH_MAX );
			p = proppath + sg_strlcpy( proppath, dirname(dnbuf), PATH_MAX );
			strncpy( p, "/libstatgrab-test.properties", PATH_MAX - ( p - proppath ) );
			rc = access( proppath, R_OK );
		}
		if( rc < 1 ) {
			char dnbuf[PATH_MAX];
			strncpy( dnbuf, argv[0], PATH_MAX );
			p = proppath + sg_strlcpy( proppath, basename(dnbuf), PATH_MAX );
			strncpy( p, ".properties", PATH_MAX - ( p - proppath ) );
			rc = access( proppath, R_OK );
		}
	}
	if( rc < 1 ) {
		getcwd( proppath, sizeof(proppath) );
		sg_strlcat( proppath, "/libstatgrab-test.properties", PATH_MAX );
		rc = access( proppath, R_OK );
	}
	if( 0 == rc ) {
		if( 0 != log4cplus_file_configure( proppath ) ) {
			char buf[4096];
			p = buf + sg_strlcpy( buf, "Can't setup log4cplus using ", 4096 );
			strncpy( p, proppath, 4096 - ( p - buf ) );
			die( ENOENT, buf );
		}
	}
#else
	((void)argc);
	((void)argv);
#endif
}
