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

#include "helpers.h"

#include <stdio.h>
#include <statgrab.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static int quit = 0;

#if 0
static void
sig_int(int signo) {
	if( signo == SIGINT )
		quit++;
}
#endif

int main(int argc, char **argv){

	extern char *optarg;
	int c;

	int delay = 1;
	sg_cpu_percents *cpu_percent;
	sg_cpu_stats *cpu_diff_stats;

	while ((c = getopt(argc, argv, "d:")) != -1){
		switch (c){
			case 'd':
				delay = atoi(optarg);
				break;
		}
	}
#ifdef WIN32
	delay = delay * 1000;
#endif

	/* Initialise helper - e.g. logging, if any */
	hlp_init(argc, argv);

	/* Initialise statgrab */
	sg_init(1);
	/* XXX must be replaced by termios/(n)curses function ....
	if( 0 != setvbuf(stdin, NULL, _IONBF, 0) ) {
		perror("setvbuf");
		exit(1);
	} */

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != SG_ERROR_NONE) {
		sg_die("Error. Failed to drop privileges", 1);
	}

	register_sig_flagger( SIGINT, &quit );

	/* Throw away the first reading as thats averaged over the machines uptime */
	sg_snapshot();
	cpu_percent = sg_get_cpu_percents();
	if( NULL == cpu_percent )
		sg_die("Failed to get cpu stats", 1);

	/* Clear the screen ready for display the cpu usage */
	printf("\033[2J");

	while( ( ( cpu_diff_stats = sg_get_cpu_stats_diff() ) != NULL ) &&
	     ( ( cpu_percent = sg_get_cpu_percents_of(sg_last_diff_cpu_percent) ) != NULL ) ) {
		int ch;
		sg_snapshot();
		printf("\033[2;2H%-14s : %lld (%6.2f)", "User CPU", cpu_diff_stats->user, cpu_percent->user);
		printf("\033[3;2H%-14s : %lld (%6.2f)", "Kernel CPU", cpu_diff_stats->kernel, cpu_percent->kernel);
		printf("\033[4;2H%-14s : %lld (%6.2f)", "IOWait CPU", cpu_diff_stats->iowait, cpu_percent->iowait);
		printf("\033[5;2H%-14s : %lld (%6.2f)", "Swap CPU", cpu_diff_stats->swap, cpu_percent->swap);
		printf("\033[6;2H%-14s : %lld (%6.2f)", "Nice CPU", cpu_diff_stats->nice, cpu_percent->nice);
		printf("\033[7;2H%-14s : %lld (%6.2f)", "Idle CPU", cpu_diff_stats->idle, cpu_percent->idle);
		printf("\033[8;2H%-14s : %llu", "Ctxts", cpu_diff_stats->context_switches);
		printf("\033[9;2H%-14s : %llu", "  Voluntary", cpu_diff_stats->voluntary_context_switches);
		printf("\033[10;2H%-14s : %llu", "  Involuntary", cpu_diff_stats->involuntary_context_switches);
		printf("\033[11;2H%-14s : %llu", "Syscalls", cpu_diff_stats->syscalls);
		printf("\033[12;2H%-14s : %llu", "Intrs", cpu_diff_stats->interrupts);
		printf("\033[13;2H%-14s : %llu", "SoftIntrs", cpu_diff_stats->soft_interrupts);
		fflush(stdout);
		ch = inp_wait(delay);
		if( quit || (ch == 'q') )
			break;
	}
	sg_shutdown();

	exit(0);
}
