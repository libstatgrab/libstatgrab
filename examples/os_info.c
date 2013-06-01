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

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#endif

static const char *host_states[] = {
	"unknown", "physical host",
	"virtual machine (full virtualized)",
	"virtual machine (paravirtualized)",
	"hardware virtualization"
};

int main(int argc, char **argv){

	sg_host_info *general_stats;

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	general_stats = sg_get_host_info();
	if(general_stats == NULL)
		sg_die("Failed to get os stats", 1);

	printf("OS name : %s\n", general_stats->os_name);
	printf("OS release : %s\n", general_stats->os_release);
	printf("OS version : %s\n", general_stats->os_version);
	printf("Hardware platform : %s\n", general_stats->platform);
	printf("Machine nodename : %s\n", general_stats->hostname);
	printf("Machine uptime : %lld\n", (long long)general_stats->uptime);
	printf("Number of CPU's configured : %u\n", general_stats->maxcpus);
	printf("Number of CPU's online : %u\n", general_stats->ncpus);
	printf("Kernel bitwidth : %u\n", general_stats->bitwidth);
	printf("Host state : %s\n", ((size_t)general_stats->host_state) > (lengthof(host_states) - 1)
	                            ? "unexpected state (libstatgrab to new)"
	                            : host_states[general_stats->host_state] );

	exit(0);
}
