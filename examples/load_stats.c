/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

static int quit;

int main(int argc, char **argv){

	extern char *optarg;
	int c;

	int delay = 1;
	sg_load_stats *load_stat;

	while ((c = getopt(argc, argv, "d:")) != -1){
		switch (c){
			case 'd':
				delay = atoi(optarg);
				break;
		}
	}

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	register_sig_flagger( SIGINT, &quit );

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	if ((load_stat = sg_get_load_stats()) == NULL)
		sg_die("Failed to get load stats", 1);

	while( (load_stat = sg_get_load_stats()) != NULL){
		int ch;
		printf("Load 1 : %5.2f\t Load 5 : %5.2f\t Load 15 : %5.2f\n", load_stat->min1, load_stat->min5, load_stat->min15);
		ch = inp_wait(delay);
		if( quit || (ch == 'q') )
			break;
	}

	exit(0);
}
