/*
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
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

int main(int argc, char **argv){

	extern char *optarg;
	extern int optind;
	int c;

	/* We default to 1 second updates and displaying in bytes*/
	int delay = 1;
	char units = 'b';

	diskio_stat_t *diskio_stats;
	int num_diskio_stats;

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
				break;
			case 'm':
				units = 'm';
				break;
		}
	}

	/* Initialise statgrab */
	statgrab_init();

	/* Drop setuid/setgid privileges. */
	if (statgrab_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	/* We are not interested in the amount of traffic ever transmitted, just differences. 
	 * Because of this, we do nothing for the very first call.
	 */

	diskio_stats = get_diskio_stats_diff(&num_diskio_stats);
	if (diskio_stats == NULL){
		perror("Error. Failed to get disk stats");
		return 1;
	}

	/* Clear the screen ready for display the disk stats */
	printf("\033[2J");

	/* Keep getting the disk stats */
	while ( (diskio_stats = get_diskio_stats_diff(&num_diskio_stats)) != NULL){
		int x;
		int line_number = 2;

		long long total_write=0;
		long long total_read=0;

		for(x = 0; x < num_diskio_stats; x++){	
			/* Print at location 2, linenumber the interface name */
			printf("\033[%d;2H%-25s : %-10s", line_number++, "Disk Name", diskio_stats->disk_name);
			/* Print out at the correct location the traffic in the requsted units passed at command time */
			switch(units){
				case 'b':
					printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk read", diskio_stats->read_bytes);
					printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk write", diskio_stats->write_bytes);
					break;
				case 'k':
					printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk read", (diskio_stats->read_bytes / 1024));
					printf("\033[%d;2H%-25s : %5lld", line_number++, "Disk write", (diskio_stats->write_bytes / 1024));
					break;
				case 'm':
					printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk read", diskio_stats->read_bytes / (1024.00*1024.00));
					printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk write", diskio_stats->write_bytes / (1024.00*1024.00));
			}
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
		switch(units){
			case 'b':
				printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk Total read", total_read);
				printf("\033[%d;2H%-25s : %8lld b", line_number++, "Disk Total write", total_write);
				break;
			case 'k':
				printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk Total read", (total_read / 1024));
				printf("\033[%d;2H%-25s : %5lld k", line_number++, "Disk Total write", (total_write / 1024));
				break;
			case 'm':
				printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk Total read", (total_read  / (1024.00*1024.00)));
				printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Disk Total write", (total_write  / (1024.00*1024.00)));
				break;
		}

		fflush(stdout);

		sleep(delay);

	}

	return 0;

}
