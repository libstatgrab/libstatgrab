/*
 * libstatgrab
 * https://libstatgrab.org
 * Copyright (C) 2003-2004 Peter Saunders
 * Copyright (C) 2003-2019 Tim Bishop
 * Copyright (C) 2003-2013 Adam Sampson
 * Copyright (C) 2012-2019 Jens Rehsack
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

int main(int argc, char **argv){

	sg_mem_stats *mem_stats;
	sg_swap_stats *swap_stats;

	long long total, free;

#ifdef HAVE_LOG4CPLUS_INITIALIZE
	l4cplus_initializer = log4cplus_initialize();
	atexit((void (*)(void))cleanup_logging);
#endif

	/* Initialise helper - e.g. logging, if any */
	sg_log_init("libstatgrab-examples", "SGEXAMPLES_LOG_PROPERTIES", argc ? argv[0] : NULL);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	if( ((mem_stats = sg_get_mem_stats(NULL)) != NULL) &&
	    ((swap_stats = sg_get_swap_stats(NULL)) != NULL) ) {
		printf("Total memory in bytes : %llu\n", mem_stats->total);
		printf("Used memory in bytes : %llu\n", mem_stats->used);
		printf("Cache memory in bytes : %llu\n", mem_stats->cache);
		printf("Free memory in bytes : %llu\n", mem_stats->free);

		printf("Swap total in bytes : %llu\n", swap_stats->total);
		printf("Swap used in bytes : %llu\n", swap_stats->used);
		printf("Swap free in bytes : %llu\n", swap_stats->free);

		total = mem_stats->total + swap_stats->total;
		free = mem_stats->free + swap_stats->free;

		printf("Total VM usage : %5.2f%%\n", 100 - (100*((float)free/(float)total)));
	}
	else {
		sg_die("Unable to get VM stats", 1);
	}

	exit(0);
}
