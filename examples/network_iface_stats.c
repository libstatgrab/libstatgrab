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
 * $Id
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv){

	sg_network_iface_stats *network_iface_stats;
	size_t iface_count, i;

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	network_iface_stats = sg_get_network_iface_stats(&iface_count);
	if(network_iface_stats == NULL)
		sg_die("Failed to get network interface stats", 1);

	if (argc != 1) {
		/* If an argument is given, use bsearch to find just that
		 * interface. */
		sg_network_iface_stats key;

		key.interface_name = argv[1];
		network_iface_stats = bsearch(&key, network_iface_stats,
		                              iface_count,
		                              sizeof *network_iface_stats,
		                              sg_network_iface_compare_name);
		if (network_iface_stats == NULL) {
			fprintf(stderr, "Interface %s not found\n", argv[1]);
			exit(1);
		}
		iface_count = 1;
	}

	printf("Name\tSpeed\tDuplex\n");
	for(i = 0; i < iface_count; i++) {
		printf("%s\t%d\t", network_iface_stats->interface_name, network_iface_stats->speed);
		switch (network_iface_stats->duplex) {
		case SG_IFACE_DUPLEX_FULL:
			printf("full\n");
			break;
		case SG_IFACE_DUPLEX_HALF:
			printf("half\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		network_iface_stats++;
	}

	exit(0);
}
