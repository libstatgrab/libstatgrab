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

/* A very basic example of how to get the disk statistics from the system
 * and diaply them. Also it adds up all the traffic to create and print out
 * a total
 * Takes several arguments :
 * -d <number>	Takes the number of seconds to wait to get traffic sent since last call
 * 		Note, this is not disk traffic per second. Its the traffic since last
 * 		call. If you want it traffic per second averaged over time since last call
 * 		You need to divide against the time since last time this was called. The time
 * 		between 2 calls is stored in systime in the disk_stat_t structure.
 * -b		Display in bytes
 * -k		Display in kilobytes
 * -m		Display in megabytes
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

	/* We default to 1 second updates and displaying in bytes*/
	int delay = 1;
	char units = 'b';
	unsigned long long divider = 1;

	sg_disk_io_stats *diskio_stats;
	size_t num_diskio_stats;

	/* Parse command line options */
	while ((c = getopt(argc, argv, "d:bkm")) != -1){
		switch (c){
			case 'd':
				delay =	atoi(optarg);
				break;
			case 'b':
				units = 'b';
				break;
			case 'k':
				units = 'k';
				divider = 1024;
				break;
			case 'm':
				units = 'm';
				divider = 1024 * 1024;
				break;
		}
	}

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	/* We are not interested in the amount of traffic ever transmitted, just differences.
	 * Because of this, we do nothing for the very first call.
	 */

	register_sig_flagger( SIGINT, &quit );

	diskio_stats = sg_get_disk_io_stats_diff(&num_diskio_stats);
	if (diskio_stats == NULL)
		sg_die("Failed to get disk stats", 1);

	/* Clear the screen ready for display the disk stats */
	printf("\033[2J");

	/* Keep getting the disk stats */
	while ( (diskio_stats = sg_get_disk_io_stats_diff(&num_diskio_stats)) != NULL) {
		size_t x;
		int line_number = 2;
		int ch;

		long long total_write=0;
		long long total_read=0;

		qsort(diskio_stats , num_diskio_stats, sizeof(diskio_stats[0]), sg_disk_io_compare_traffic);

		for(x = 0; x < num_diskio_stats; x++){
			/* Print at location 2, linenumber the interface name */
			printf("\033[%d;2H%-25s : %-10s", line_number++, "Disk Name", diskio_stats->disk_name);
			/* Print out at the correct location the traffic in the requsted units passed at command time */
			printf("\033[%d;2H%-25s : %8llu %c", line_number++, "Disk read", diskio_stats->read_bytes / divider, units);
			printf("\033[%d;2H%-25s : %8llu %c", line_number++, "Disk write", diskio_stats->write_bytes / divider, units);
			printf("\033[%d;2H%-25s : %ld ", line_number++, "Disk systime", (long) diskio_stats->systime);

			/* Add a blank line between interfaces */
			line_number++;

			/* Add up this interface to the total so we can display a "total" disk io" */
			total_write+=diskio_stats->write_bytes;
			total_read+=diskio_stats->read_bytes;

			/* Move the pointer onto the next interface. Since this returns a static buffer, we dont need
			 * to keep track of the orginal pointer to free later */
			diskio_stats++;
		}

		printf("\033[%d;2H%-25s : %-10s", line_number++, "Disk Name", "Total Disk IO");
		printf("\033[%d;2H%-25s : %8llu %c", line_number++, "Disk Total read", total_read / divider, units);
		printf("\033[%d;2H%-25s : %8llu %c", line_number++, "Disk Total write", total_write / divider, units);

		fflush(stdout);

		ch = inp_wait(delay);
		if( quit || (ch == 'q') )
			break;
	}

	return 0;

}
