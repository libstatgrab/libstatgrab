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

static int quit;

int main(int argc, char **argv){

	extern char *optarg;
	int c;

	int delay = 1;
	sg_page_stats *page_stats;

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

	page_stats = sg_get_page_stats_diff();
	if(page_stats == NULL)
		sg_die("Failed to get page stats", 1);

	while( (page_stats = sg_get_page_stats_diff()) != NULL){
		int ch;
		printf("Pages in : %llu\n", page_stats->pages_pagein);
		printf("Pages out : %llu\n", page_stats->pages_pageout);
		ch = inp_wait(delay);
		if( quit || (ch == 'q') )
			break;
	}

	exit(0);
}
