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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/termios.h>
#include <signal.h>
#include <errno.h>
#include <statgrab.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>

#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

typedef struct{
	cpu_percent_t *cpu_percents;
	mem_stat_t *mem_stats;
	swap_stat_t *swap_stats;
	load_stat_t *load_stats;
	process_stat_t *process_stats;
	page_stat_t *page_stats;

	network_stat_t *network_stats;
	int network_entries;

	diskio_stat_t *diskio_stats;
	int diskio_entries;

	disk_stat_t *disk_stats;
	int disk_entries;

	general_stat_t *general_stats;
	user_stat_t *user_stats;
}stats_t;
	
stats_t stats;

char *size_conv(long long number){
	char type[] = {'B', 'K', 'M', 'G', 'T'};
	int x=0;
	static char string[10];

	for(;x<5;x++){
		if( (number/1024) < (100)) {
			break;
		}
		number = (number/1024);
	}

	snprintf(string, 10, "%lld%c", number, type[x]);	
	return string;
	
}

char *hr_uptime(time_t time){
	int day = 0, hour = 0, min = 0;
	static char uptime_str[25];
	int sec = (int) time;
	
	day = sec / (24*60*60);
	sec = sec % (24*60*60);
	hour = sec / (60*60);
	sec = sec % (60*60);
	min = sec / 60;
	sec = sec % 60;

	if(day){
		snprintf(uptime_str, 25, "%dd %02d:%02d:%02d", day, hour, min, sec);
	}else{
		snprintf(uptime_str, 25, "%02d:%02d:%02d", hour, min, sec);
	}
	return uptime_str;
}

void display_headings(){
	int line;

	move(0,0);
	printw("Hostname  :");
	move(0,27);
	printw("Uptime : ");
	move(0,54);
	printw("Date : ");

	/* Load */
	move(2,0);
	printw("Load 1    :");
	move(3,0);
	printw("Load 5    :");
	move(4,0);
	printw("Load 15   :");

	/* CPU */
	move(2,21);
	printw("CPU Idle  :");
	move(3,21);
	printw("CPU System:");
	move(4,21);
	printw("CPU User  :");

	/* Process */
	move(2, 42);
	printw("Running   :");
	move(3, 42);
	printw("Sleeping  :");
	move(4, 42);
	printw("Stopped   :");
	move(2, 62);
	printw("Zombie    :");
	move(3, 62);
	printw("Total     :");
	move(4, 62);
	printw("No. Users :");

	/* Mem */
	move(6, 0);
	printw("Mem Total :");
	move(7, 0);
	printw("Mem Used  :");
	move(8, 0);
	printw("Mem Free  :");

	/* Swap */
        move(6, 21);
        printw("Swap Total:");
        move(7, 21);
        printw("Swap Used :");
        move(8, 21);
        printw("Swap Free :");

	/* VM */
	move(6, 42);
	printw("Mem Used  :");
	move(7, 42);
	printw("Swap Used :");
	move(8, 42);
	printw("Total Used:");

	/* Paging */
	move(6, 62);
	printw("Paging in :");
	move(7, 62);
	printw("Paging out:");

	/* Disk IO */
	move(10,0);
	printw("Disk Name");
	move(10,15);
	printw("Read");
	move(10,28);
	printw("Write");	

	line = 10;
	if (stats.network_stats != NULL) {
		/* Network IO */
		move(line, 42);
		printw("Network Interface");
		move(line, 67);
		printw("rx");
		move(line, 77);
		printw("tx");
		line += 2 + stats.network_entries;
	}

	move(line, 42);
	printw("Mount Point");
	move(line, 65);
	printw("Free");
	move(line, 75);
	printw("Used");

	refresh();
}

void display_data(){
	char cur_time[20];
	struct tm *tm_time;
	time_t epoc_time;
	int counter, line;
	long long r,w;
	long long rt, wt;
	diskio_stat_t *diskio_stat_ptr;
	network_stat_t *network_stat_ptr;
	disk_stat_t *disk_stat_ptr;
	/* Size before it will start overwriting "uptime" */
	char hostname[15];
	char *ptr;

	if (stats.general_stats != NULL) {
		move(0,12);
		strncpy(hostname, stats.general_stats->hostname, (sizeof(hostname) - 1));
		/* strncpy does not NULL terminate.. If only strlcpy was on all platforms :) */
		hostname[14] = '\0';
		ptr=strchr(hostname, '.');
		/* Some hosts give back a FQDN for hostname. To avoid this, we'll
		 * just blank out everything after the first "."
		 */
		if (ptr != NULL){
			*ptr = '\0';
		}	
		printw("%s", hostname);
		move(0,36);
		printw("%s", hr_uptime(stats.general_stats->uptime));
		epoc_time=time(NULL);
		tm_time = localtime(&epoc_time);
		strftime(cur_time, 20, "%Y-%m-%d %T", tm_time);
		move(0,61);
		printw("%s", cur_time);
	}

	if (stats.load_stats != NULL) {
		/* Load */
		move(2,12);
		printw("%6.2f", stats.load_stats->min1);
		move(3,12);
		printw("%6.2f", stats.load_stats->min5);
		move(4,12);
		printw("%6.2f", stats.load_stats->min15);
	}

	if (stats.cpu_percents != NULL) {
		/* CPU */
		move(2,33);
		printw("%6.2f%%", stats.cpu_percents->idle);
		move(3,33);
		printw("%6.2f%%", (stats.cpu_percents->kernel + stats.cpu_percents->iowait + stats.cpu_percents->swap));
		move(4,33);
		printw("%6.2f%%", (stats.cpu_percents->user + stats.cpu_percents->nice));
	}

	if (stats.process_stats != NULL) {
		/* Process */
		move(2, 54);
		printw("%5d", stats.process_stats->running);
		move(2,74);
		printw("%5d", stats.process_stats->zombie);
		move(3, 54);
		printw("%5d", stats.process_stats->sleeping);
		move(3, 74);
		printw("%5d", stats.process_stats->total);
		move(4, 54);
		printw("%5d", stats.process_stats->stopped);
	}
	if (stats.user_stats != NULL) {
		move(4,74);
		printw("%5d", stats.user_stats->num_entries);
	}

	if (stats.mem_stats != NULL) {
		/* Mem */
		move(6, 12);
		printw("%7s", size_conv(stats.mem_stats->total));	
		move(7, 12);
		printw("%7s", size_conv(stats.mem_stats->used));	
		move(8, 12);
		printw("%7s", size_conv(stats.mem_stats->free));
	}
	
	if (stats.swap_stats != NULL) {	
		/* Swap */
		move(6, 32);
		printw("%8s", size_conv(stats.swap_stats->total));
		move(7, 32);
		printw("%8s", size_conv(stats.swap_stats->used));
		move(8, 32);
		printw("%8s", size_conv(stats.swap_stats->free));
	}

	/* VM */
	if (stats.mem_stats != NULL && stats.mem_stats->total != 0) {	
		move(6, 54);
		printw("%5.2f%%", (100.00 * (float)(stats.mem_stats->used)/stats.mem_stats->total));
	}
	if (stats.swap_stats != NULL && stats.swap_stats->total != 0) {	
		move(7, 54);
		printw("%5.2f%%", (100.00 * (float)(stats.swap_stats->used)/stats.swap_stats->total));
	}
	if (stats.mem_stats != NULL && stats.swap_stats != NULL &&
	    stats.mem_stats->total != 0 && stats.swap_stats->total != 0) {	
		move(8, 54);
		printw("%5.2f%%", (100.00 * (float)(stats.mem_stats->used+stats.swap_stats->used)/(stats.mem_stats->total+stats.swap_stats->total)));
	}

	if (stats.page_stats != NULL) {	
		/* Paging */
		move(6, 74);
		printw("%5d", (stats.page_stats->systime)? (stats.page_stats->pages_pagein / stats.page_stats->systime): stats.page_stats->pages_pagein);
		move(7, 74);
		printw("%5d", (stats.page_stats->systime)? (stats.page_stats->pages_pageout / stats.page_stats->systime) : stats.page_stats->pages_pageout);
	}

	line = 11;
	if (stats.diskio_stats != NULL) {	
		/* Disk IO */
		diskio_stat_ptr = stats.diskio_stats;
		r=0;
		w=0;
		for(counter=0;counter<stats.diskio_entries;counter++){
			move(line, 0);
			printw("%s", diskio_stat_ptr->disk_name);
			move(line, 12);
			rt = (diskio_stat_ptr->systime)? (diskio_stat_ptr->read_bytes/diskio_stat_ptr->systime): diskio_stat_ptr->read_bytes;
			printw("%7s", size_conv(rt));
			r+=rt;
			move(line, 26);
			wt = (diskio_stat_ptr->systime)? (diskio_stat_ptr->write_bytes/diskio_stat_ptr->systime): diskio_stat_ptr->write_bytes;
			printw("%7s", size_conv(wt));
			w+=wt;
			diskio_stat_ptr++;
			line++;
		}
		line++;
		move(line, 0);
		printw("Total");
		move(line, 12);
		printw("%7s", size_conv(r));
		move(line, 26);
		printw("%7s", size_conv(w));
	}

	line = 11;
	if (stats.network_stats != NULL) {	
		/* Network */
		network_stat_ptr = stats.network_stats;
		for(counter=0;counter<stats.network_entries;counter++){
	                move(line, 42);
			printw("%s", network_stat_ptr->interface_name);
			move(line, 62);
			rt = (network_stat_ptr->systime)? (network_stat_ptr->rx / network_stat_ptr->systime): network_stat_ptr->rx;
			printw("%7s", size_conv(rt));
			move(line, 72);
			wt = (network_stat_ptr->systime)? (network_stat_ptr->tx / network_stat_ptr->systime): network_stat_ptr->tx; 
			printw("%7s", size_conv(wt));
			network_stat_ptr++;
			line++;
		}
		line += 2;
	}

	if (stats.disk_stats != NULL) {	
		/* Disk */
		disk_stat_ptr = stats.disk_stats;
		for(counter=0;counter<stats.disk_entries;counter++){
			move(line, 42);
			printw("%s", disk_stat_ptr->mnt_point);
			move(line, 62);
			printw("%7s", size_conv(disk_stat_ptr->avail));
			move(line, 73);
			printw("%5.2f%%", 100.00 * ((float) disk_stat_ptr->used / (float) (disk_stat_ptr->used + disk_stat_ptr->avail)));
			disk_stat_ptr++;
			line++;
		}
	}

	refresh();
}

void sig_winch_handler(int sig){
	clear();
	display_headings();
	display_data();
        signal(SIGWINCH, sig_winch_handler);
}

int get_stats(){
	stats.cpu_percents = cpu_percent_usage();
	stats.mem_stats = get_memory_stats();
	stats.swap_stats = get_swap_stats();
	stats.load_stats = get_load_stats();
	stats.process_stats = get_process_stats();
	stats.page_stats = get_page_stats_diff();
	stats.network_stats = get_network_stats_diff(&(stats.network_entries));
	stats.diskio_stats = get_diskio_stats_diff(&(stats.diskio_entries));
	stats.disk_stats = get_disk_stats(&(stats.disk_entries));
	stats.general_stats = get_general_stats();
	stats.user_stats = get_user_stats();

	return 1;
}

void version_num(char *progname){
	fprintf(stderr, "%s version %s\n", progname, PACKAGE_VERSION);
	fprintf(stderr, "\nReport bugs to <%s>.\n", PACKAGE_BUGREPORT);
	exit(1);
}

void usage(char *progname){
        fprintf(stderr, "Usage: %s [-d delay] [-v] [-h]\n\n", progname);
        fprintf(stderr, "  -d    Sets the update time in seconds\n");
	fprintf(stderr, "  -v    Prints version number\n");
        fprintf(stderr, "  -h    Displays this help information.\n");
        fprintf(stderr, "\nReport bugs to <%s>.\n", PACKAGE_BUGREPORT);
        exit(1);
}

int main(int argc, char **argv){

        extern char *optarg;
        extern int optind;
        int c;

	time_t last_update = 0;

	WINDOW *window;

	extern int errno;

	int delay=2;

	statgrab_init();
	if(statgrab_drop_privileges() != 0){
		fprintf(stderr, "Failed to drop setuid/setgid privileges\n");
		return 1;
	}
		
        while ((c = getopt(argc, argv, "vhd:")) != -1){
                switch (c){
                        case 'd':
                                delay = atoi(optarg);
				if (delay < 1){
					fprintf(stderr, "Time must be 1 second or greater\n");
					exit(1);
				}
                                break;
			case 'v':
				version_num(argv[0]);	
				break;
			case 'h':
			default:
				usage(argv[0]);
				return 1;
				break;
                }
        }

	signal(SIGWINCH, sig_winch_handler);
        initscr();
        nonl();
        cbreak();
        noecho();
	timeout(delay * 1000);
        window=newwin(0, 0, 0, 0);
	clear();

	if(!get_stats()){
		fprintf(stderr, "Failed to get all the stats. Please check correct permissions\n");
		endwin();
		return 1;
	}

	display_headings();

	for(;;){
		time_t now;
		int ch = getch();

		if (ch == 'q'){
			break;
		}

		/* To keep the numbers slightly accurate we do not want them
		 * updating more frequently than once a second.
		 */
		now = time(NULL);
		if ((now - last_update) >= 1) {
			get_stats();
		}
		last_update = now;

		display_data();
	}	

	endwin();
	return 0;
}	
