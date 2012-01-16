/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

int main(int argc, char **argv){
	sg_process_stats *ps;
	size_t ps_size;
	size_t x;
	char *state = NULL;

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE)
		sg_die("Error. Failed to drop privileges", 1);

	ps = sg_get_process_stats(&ps_size);
	if(ps == NULL)
		sg_die("Failed to get process snapshot", 1);

	qsort(ps, ps_size, sizeof *ps, sg_process_compare_pid);

	printf("%5s %5s %5s %5s %5s %5s %5s %6s %6s %9s %-10s %-4s %-8s %-6s %-6s %-20s %s\n",
		"pid", "ppid", "pgid", "uid", "euid", "gid", "egid", "size", "res", "time", "cpu", "nice", "state", "nvcsw", "nivcsw", "name", "title");

	for(x=0;x<ps_size;x++){
		switch (ps->state) {
		case SG_PROCESS_STATE_RUNNING:
			state = "RUNNING";
			break;
		case SG_PROCESS_STATE_SLEEPING:
			state = "SLEEPING";
			break;
		case SG_PROCESS_STATE_STOPPED:
			state = "STOPPED";
			break;
		case SG_PROCESS_STATE_ZOMBIE:
			state = "ZOMBIE";
			break;
		case SG_PROCESS_STATE_UNKNOWN:
		default:
			state = "UNKNOWN";
			break;
		}
		printf("%5u %5u %5u %5u %5u %5u %5u %5uM %5uM %8ds %10f %4d %-8s %6llu %6llu %-20s %s\n",
			(unsigned)ps->pid, (unsigned)ps->parent, (unsigned)ps->pgid,
			(unsigned)ps->uid, (unsigned)ps->euid, (unsigned)ps->gid, (unsigned)ps->egid,
			(unsigned)(ps->proc_size / (1024*1024)),
			(unsigned)(ps->proc_resident / (1024*1024)),
			(unsigned)ps->time_spent, (float)ps->cpu_percent,
			ps->nice, state,
			ps->voluntary_context_switches,
			ps->involuntary_context_switches,
			ps->process_name, ps->proctitle);
		ps++;
	}
	return 0;
}
