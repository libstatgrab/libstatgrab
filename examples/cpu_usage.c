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

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){

	extern char *optarg;
	int c;

	int delay = 1;
	cpu_percent_t *cpu_percent;

	while ((c = getopt(argc, argv, "d:")) != -1){
		switch (c){
			case 'd':
				delay = atoi(optarg);
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

	/* Throw away the first reading as thats averaged over the machines uptime */
	cpu_percent = cpu_percent_usage();

	/* Clear the screen ready for display the cpu usage */
	printf("\033[2J");

	while( (cpu_percent = cpu_percent_usage()) != NULL){
		printf("\033[2;2H%-12s : %6.2f", "User CPU", cpu_percent->user);
		printf("\033[3;2H%-12s : %6.2f", "Kernel CPU", cpu_percent->kernel);
		printf("\033[4;2H%-12s : %6.2f", "IOWait CPU", cpu_percent->iowait);
		printf("\033[5;2H%-12s : %6.2f", "Swap CPU", cpu_percent->swap);
		printf("\033[6;2H%-12s : %6.2f", "Nice CPU", cpu_percent->nice);
		printf("\033[7;2H%-12s : %6.2f", "Idle CPU", cpu_percent->idle);
		fflush(stdout);
		sleep(delay);
	}

	exit(0);
}
