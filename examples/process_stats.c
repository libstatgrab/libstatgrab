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
	
	extern char *optarg;
        extern int optind;
        int c;

	int delay = 1;
	process_stat_t *process_stat;

	while ((c = getopt(argc, argv, "d:")) != -1){
                switch (c){
                        case 'd':
                                delay = atoi(optarg);
                                break;
		}
	}

	while( (process_stat = get_process_stats()) != NULL){
		printf("Running:%3d \t", process_stat->running);
		printf("Sleeping:%5d \t",process_stat->sleeping);
		printf("Stopped:%4d \t", process_stat->stopped);
		printf("Zombie:%4d \t", process_stat->zombie);
		printf("Total:%5d\n", process_stat->total);
		sleep(delay);
	}

	exit(0);
}



