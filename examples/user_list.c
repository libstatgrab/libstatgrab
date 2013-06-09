/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
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

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv){

	size_t nusers, x;
	sg_user_stats *users;

#if 0
	int c;
	int delay = 1;

	while ((c = getopt(argc, argv, "d:")) != -1){
		switch (c){
			case 'd':
				delay = atoi(optarg);
				break;
		}
	}
#endif
	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	users = sg_get_user_stats(&nusers);
	if( users == NULL )
		sg_die("Failed to get logged on users", 1);

	printf( "%16s %16s %24s %8s %24s\n", "login name", "device", "hostname", "pid", "login time" );

	for( x = 0; x < nusers; ++x ) {
		char ltbuf[256];
		struct tm *tm;

		tm = localtime(&users[x].login_time);
		strftime(ltbuf, sizeof(ltbuf), "%c", tm);
		printf( "%16s %16s %24s %8d %24s\n", users[x].login_name, users[x].device, users[x].hostname, (int) users[x].pid, ltbuf );
	}

	exit(0);
}
