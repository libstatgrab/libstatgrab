/*
 * i-scream central monitoring system
 * http://www.i-scream.org.uk
 * Copyright (C) 2000-2002 i-scream
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

/* A very basic example of how to get the network statistics from the system 
 * and diaply them. Also it adds up all the traffic to create and print out
 * a total 
 * Takes several arguments :
 * -d <number> 	Takes the number of seconds to wait to get traffic sent since last call
 *              Note, this is not network traffic per second. Its the traffic since last
 * 	  	call. If you want it traffic per second averaged over time since last call
 * 		You need to divide against the time since last time this was called. The time
 * 		between 2 calls is stored in systime in the network_stat_t structure.
 * -b		Display in bytes
 * -k		Display in kilobytes
 * -m 		Display in megabytes
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

	network_stat_t *network_stats;
	int num_network_stats;

	/* Parse command line options */
	while ((c = getopt(argc, argv, "d:bkm")) != EOF){
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

	/* We are not intrested in the amount of traffic ever transmitted, just differences. 
	 * Because of this, we do nothing for the very first call.
	 */
	
	network_stats = get_network_stats_diff(&num_network_stats);
	if (network_stats == NULL){
		perror("Error. Failed to get network stats");
		return 1;
	}

	/* Clear the screen ready for display the network stats */
	printf("\033[2J");
	
	/* Keep getting the network stats */
	while ( (network_stats = get_network_stats_diff(&num_network_stats)) != NULL){
		int x;
		int line_number = 2;

		long long total_tx=0;
		long long total_rx=0;

		for(x = 0; x < num_network_stats; x++){	
			/* Print at location 2, linenumber the interface name */
			printf("\033[%d;2H%-25s : %-10s", line_number++, "Network Interface Name", network_stats->interface_name);
			/* Print out at the correct location the traffic in the requsted units passed at command time */
			switch(units){
				case 'b':
					printf("\033[%d;2H%-25s : %8lld b", line_number++, "Network Interface Rx", network_stats->rx);
					printf("\033[%d;2H%-25s : %8lld b", line_number++, "Network Interface Tx", network_stats->tx);
					break;
				case 'k':
					printf("\033[%d;2H%-25s : %5lld k", line_number++, "Network Interface Rx", (network_stats->rx / 1024));
                                        printf("\033[%d;2H%-25s : %5lld", line_number++, "Network Interface Tx", (network_stats->tx / 1024));
                                        break;
				case 'm':
					printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Network Interface Tx", network_stats->rx / (1024.00*1024.00));
					printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Network Interface Tx", network_stats->tx / (1024.00*1024.00));
			}
			printf("\033[%d;2H%-25s : %ld ", line_number++, "Network Interface systime", (long) network_stats->systime);
		
			/* Add a blank line between interfaces */	
			line_number++;

			/* Add up this interface to the total so we can display a "total" network io" */
			total_tx+=network_stats->tx;
			total_rx+=network_stats->rx;

			/* Move the pointer onto the next interface. Since this returns a static buffer, we dont need
			 * to keep track of the orginal pointer to free later */
			network_stats++;
		}
		
		printf("\033[%d;2H%-25s : %-10s", line_number++, "Network Interface Name", "Total Network IO");
		switch(units){
			case 'b':
				printf("\033[%d;2H%-25s : %8lld b", line_number++, "Network Total Rx", total_rx);
				printf("\033[%d;2H%-25s : %8lld b", line_number++, "Network Total Tx", total_tx);
				break;
			case 'k':
				printf("\033[%d;2H%-25s : %5lld k", line_number++, "Network Total Rx", (total_rx / 1024));
				printf("\033[%d;2H%-25s : %5lld k", line_number++, "Network Total Tx", (total_tx / 1024));
				break;
			case 'm':
				printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Network Total Rx", (total_rx  / (1024.00*1024.00)));
				printf("\033[%d;2H%-25s : %5.2f m", line_number++, "Network Total Tx", (total_tx  / (1024.00*1024.00)));
				break;
		}
		
		fflush(stdout);

		sleep(delay);

	}

	return 0;

}
