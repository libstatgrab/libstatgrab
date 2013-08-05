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

/* A very basic example of how to get the filesystem statistics from the
 * system and display them.
 */

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

int main(int argc, char **argv){
	sg_fs_stats *fs_stats;
	size_t fs_size;

	/* Initialise helper - e.g. logging, if any */
	sg_log_init("libstatgrab-examples", "SGEXAMPLES_LOG_PROPERTIES", argc ? argv[0] : NULL);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	fs_stats = sg_get_fs_stats(&fs_size);
	if(fs_stats == NULL)
		sg_die("Failed to get file systems snapshot", 1);

	printf( "%-16s %-24s %-8s %16s %16s %16s %7s %7s %7s %7s %9s %9s %7s %7s %7s %7s\n",
		"device", "mountpt", "fstype", "size", "used", "avail",
		"i-total", "i-used", "i-free", "i-avail",
		"io size", "block size",
		"b-total", "b-used", "b-free", "b-avail");

	for( ; fs_size > 0 ; --fs_size, ++fs_stats ) {
		printf( "%-16s %-24s %-8s %16llu %16llu %16llu %7llu %7llu %7llu %7llu %9llu %9llu %7llu %7llu %7llu %7llu\n",
			fs_stats->device_name, fs_stats->mnt_point, fs_stats->fs_type,
			fs_stats->size, fs_stats->used, fs_stats->avail,
			fs_stats->total_inodes, fs_stats->used_inodes, fs_stats->free_inodes, fs_stats->avail_inodes,
			fs_stats->io_size, fs_stats->block_size,
			fs_stats->total_blocks, fs_stats->used_blocks, fs_stats->free_blocks, fs_stats->avail_blocks);
	}

	return 0;
}
