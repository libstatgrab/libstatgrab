/*
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2003 i-scream
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
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){

	general_stat_t *general_stats;

	/* Initialise statgrab */
	statgrab_init();

	general_stats = get_general_stats();

	if(general_stats == NULL){
		fprintf(stderr, "Failed to get os stats\n");
		exit(1);
	}

	printf("OS name : %s\n", general_stats->os_name);
	printf("OS release : %s\n", general_stats->os_release);
	printf("OS version : %s\n", general_stats->os_version);
	printf("Hardware platform : %s\n", general_stats->platform);
	printf("Machine nodename : %s\n", general_stats->hostname);
	printf("Machine uptime : %lld\n", (long long)general_stats->uptime);

	exit(0);
}
