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
 * $Id
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){

	network_iface_stat_t *network_iface_stats;
	int iface_count, i;

	/* Initialise statgrab */
	statgrab_init();

	/* Drop setuid/setgid privileges. */
	if (statgrab_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	network_iface_stats = get_network_iface_stats(&iface_count);

	if(network_iface_stats == NULL){
		fprintf(stderr, "Failed to get network interface stats\n");
		exit(1);
	}

	printf("Name\tSpeed\tDuplex\n");
	for(i = 0; i < iface_count; i++) {
		printf("%s\t%d\t", network_iface_stats->interface_name, network_iface_stats->speed);
		switch (network_iface_stats->dup) {
		case FULL_DUPLEX:
			printf("full\n");
			break;
		case HALF_DUPLEX:
			printf("half\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
	}

	exit(0);
}
