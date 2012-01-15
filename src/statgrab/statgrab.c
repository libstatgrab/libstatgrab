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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <statgrab.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
	LONG_LONG = 0,
	UNSIGNED_LONG_LONG,
	BYTES,
	TIME_T,
	FLOAT,
	DOUBLE,
	STRING,
	INT,
	UNSIGNED,
	BOOL,
	DUPLEX
} stat_type;

typedef enum {
	DISPLAY_LINUX = 0,
	DISPLAY_BSD,
	DISPLAY_MRTG,
	DISPLAY_PLAIN
} display_mode_type;

typedef enum {
	REPEAT_NONE = 0,
	REPEAT_ONCE,
	REPEAT_FOREVER
} repeat_mode_type;

typedef struct {
	char *name;
	stat_type type;
	void *stat;
} stat_item;
/* AIX:
statgrab.c:63: error: 'stat' redeclared as different kind of symbol
/usr/include/sys/stat.h:320: error: previous declaration of 'stat' was here
 */

stat_item *stat_items = NULL;
int num_stats = 0;
int alloc_stats = 0;
#define INCREMENT_STATS 64

display_mode_type display_mode = DISPLAY_LINUX;
repeat_mode_type repeat_mode = REPEAT_NONE;
int repeat_time = 1;
int use_cpu_percent = 0;
int use_diffs = 0;
long float_scale_factor = 0;
long long bytes_scale_factor = 0;

/* Exit with an error message. */
void die(const char *s) {
	fprintf(stderr, "fatal: %s\n", s);
	exit(1);
}

/* Remove all the recorded stats. */
void clear_stats() {
	int i;

	for (i = 0; i < num_stats; i++)
		free(stat_items[i].name);
	num_stats = 0;
}

/* Add a stat. The varargs make up the name, joined with dots; the name is
   terminated with a NULL. */
void add_stat(stat_type type, void *stat, ...) {
	va_list ap;
	int len = 0;
	char *name, *p;

	/* Figure out how long the name will be, including dots and trailing
	   \0. */
	va_start(ap, stat);
	while (1) {
		const char *part = va_arg(ap, const char *);
		if (part == NULL)
			break;
		len += 1 + strlen(part);
	}
	va_end(ap);

	/* Paste the name together. */
	name = malloc(len);
	if (name == NULL)
		die("out of memory");
	p = name;
	va_start(ap, stat);
	while (1) {
		const char *part = va_arg(ap, const char *);
		int partlen;
		if (part == NULL)
			break;
		partlen = strlen(part);
		memcpy(p, part, partlen);

		/* Replace spaces and dots with underscores. */
		while (partlen-- > 0) {
			if (*p == ' ' || *p == '.')
				*p = '_';
			p++;
		}

		*p++ = '.';
	}
	va_end(ap);
	*--p = '\0';

	/* Stretch the stats array if necessary. */
	if (num_stats >= alloc_stats) {
		alloc_stats += INCREMENT_STATS;
		stat_items = realloc(stat_items, alloc_stats * sizeof *stat_items);
		if (stat_items == NULL)
			die("out of memory");
	}

	stat_items[num_stats].name = name;
	stat_items[num_stats].type = type;
	stat_items[num_stats].stat = stat;
	++num_stats;
}

/* Compare two stats by name for qsort and bsearch. */
int stats_compare(const void *a, const void *b) {
	return strcmp(((stat_item *)a)->name, ((stat_item *)b)->name);
}

/* Compare up to the length of the key for bsearch. */
int stats_compare_prefix(const void *key, const void *item) {
	const char *kn = ((stat_item *)key)->name;
	const char *in = ((stat_item *)item)->name;

	return strncmp(kn, in, strlen(kn));
}

void populate_const() {
	static int zero = 0;

	/* Constants, for use with MRTG mode. */
	add_stat(INT, &zero, "const", "0", NULL);
}

void populate_cpu() {
	sg_cpu_stats *cpu_s;

	cpu_s = use_diffs ? sg_get_cpu_stats_diff()
			  : sg_get_cpu_stats();

	if (use_cpu_percent) {
		sg_cpu_percent_source cps;
		cps = use_diffs ? sg_last_diff_cpu_percent
				: sg_entire_cpu_percent;
		sg_cpu_percents *cpu_p = sg_get_cpu_percents_of(cps);

		if (cpu_p != NULL) {
			add_stat(DOUBLE, &cpu_p->user,
				 "cpu", "user", NULL);
			add_stat(DOUBLE, &cpu_p->kernel,
				 "cpu", "kernel", NULL);
			add_stat(DOUBLE, &cpu_p->idle,
				 "cpu", "idle", NULL);
			add_stat(DOUBLE, &cpu_p->iowait,
				 "cpu", "iowait", NULL);
			add_stat(DOUBLE, &cpu_p->swap,
				 "cpu", "swap", NULL);
			add_stat(DOUBLE, &cpu_p->nice,
				 "cpu", "nice", NULL);
			add_stat(TIME_T, &cpu_p->time_taken,
				 "cpu", "time_taken", NULL);
		}
	} else {
		if (cpu_s != NULL) {
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->user,
				 "cpu", "user", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->kernel,
				 "cpu", "kernel", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->idle,
				 "cpu", "idle", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->iowait,
				 "cpu", "iowait", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->swap,
				 "cpu", "swap", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->nice,
				 "cpu", "nice", NULL);
			add_stat(UNSIGNED_LONG_LONG, &cpu_s->total,
				 "cpu", "total", NULL);
			add_stat(TIME_T, &cpu_s->systime,
				 "cpu", "systime", NULL);
		}
	}

	if (cpu_s != NULL) {
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->context_switches,
			 "cpu", "ctxsw", NULL);
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->voluntary_context_switches,
			 "cpu", "vctxsw", NULL);
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->involuntary_context_switches,
			 "cpu", "nvctxsw", NULL);
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->syscalls,
			 "cpu", "syscalls", NULL);
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->interrupts,
			 "cpu", "intrs", NULL);
		add_stat(UNSIGNED_LONG_LONG, &cpu_s->soft_interrupts,
			 "cpu", "softintrs", NULL);
	}
}

void populate_mem() {
	sg_mem_stats *mem = sg_get_mem_stats();

	if (mem != NULL) {
		add_stat(BYTES, &mem->total, "mem", "total", NULL);
		add_stat(BYTES, &mem->free, "mem", "free", NULL);
		add_stat(BYTES, &mem->used, "mem", "used", NULL);
		add_stat(BYTES, &mem->cache, "mem", "cache", NULL);
	}
}

void populate_load() {
	sg_load_stats *load = sg_get_load_stats();

	if (load != NULL) {
		add_stat(DOUBLE, &load->min1, "load", "min1", NULL);
		add_stat(DOUBLE, &load->min5, "load", "min5", NULL);
		add_stat(DOUBLE, &load->min15, "load", "min15", NULL);
	}
}

void populate_user() {
	size_t entries;
	sg_user_stats *users = sg_get_user_stats(&entries);

	if (users != NULL) {
		size_t i;
		for (i = 0; i < entries; i++) {
			const char *name = users[i].login_name;
	
			add_stat(STRING, &users[i].login_name,
				 "user", name, "login_name", NULL);
			add_stat(STRING, &users[i].device,
				 "user", name, "tty", NULL);
			add_stat(STRING, &users[i].hostname,
				 "user", name, "from", NULL);
			add_stat(TIME_T, &users[i].login_time,
				 "user", name, "login_time", NULL);
		}
	}
}

void populate_swap() {
	sg_swap_stats *swap = sg_get_swap_stats();

	if (swap != NULL) {
		add_stat(BYTES, &swap->total, "swap", "total", NULL);
		add_stat(BYTES, &swap->used, "swap", "used", NULL);
		add_stat(BYTES, &swap->free, "swap", "free", NULL);
	}
}

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#endif

static const char *host_states[] = {
        "unknown", "physical host",
        "virtual machine (full virtualized)",
        "virtual machine (paravirtualized)",
        "hardware virtualization"
};
static const char *unexpected_host_state = "unexpected state (libstatgrab to new)";

void populate_general() {
	/* FIXME this should be renamed to host. */
	sg_host_info *host = sg_get_host_info();

	if (host != NULL) {
		add_stat(STRING, &host->os_name,
			 "general", "os_name", NULL);
		add_stat(STRING, &host->os_release,
			 "general", "os_release", NULL);
		add_stat(STRING, &host->os_version,
			 "general", "os_version", NULL);
		add_stat(STRING, &host->platform, "general", "platform", NULL);
		add_stat(STRING, &host->hostname, "general", "hostname", NULL);
		add_stat(UNSIGNED, &host->ncpus, "general", "ncpus", NULL);
		add_stat(UNSIGNED, &host->maxcpus, "general", "ncpus_cfg", NULL);
		add_stat(UNSIGNED, &host->bitwidth, "general", "bitwidth", NULL);
		add_stat(STRING, ((size_t)host->host_state) > (lengthof(host_states) - 1)
		                 ? &unexpected_host_state
				 : &host_states[host->host_state], "general", "hoststate", NULL);
		add_stat(TIME_T, &host->uptime, "general", "uptime", NULL);
	}
}

void populate_fs() {
	size_t n, i;
	sg_fs_stats *disk = sg_get_fs_stats(&n);

	if (disk != NULL) {
		for (i = 0; i < n; i++) {
			/* FIXME it'd be nicer if libstatgrab did this */
			char *buf, *name, *p;
			const char *device = disk[i].device_name;

			if (strcmp(device, "/") == 0)
				device = "root";

			buf = strdup(device);
			if (buf == NULL)
				die("out of memory");

			name = buf;
			if (strlen(name) == 2 && name[1] == ':')
				name[1] = '\0';
			if (strncmp(name, "/dev/", 5) == 0)
				name += 5;
			while ((p = strchr(name, '/')) != NULL)
				*p = '_';

			add_stat(STRING, &disk[i].device_name,
				 "fs", name, "device_name", NULL);
			add_stat(STRING, &disk[i].fs_type,
				 "fs", name, "fs_type", NULL);
			add_stat(STRING, &disk[i].mnt_point,
				 "fs", name, "mnt_point", NULL);
			add_stat(BYTES, &disk[i].size,
				 "fs", name, "size", NULL);
			add_stat(BYTES, &disk[i].used,
				 "fs", name, "used", NULL);
			add_stat(BYTES, &disk[i].avail,
				 "fs", name, "avail", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].total_inodes,
				 "fs", name, "total_inodes", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].used_inodes,
				 "fs", name, "used_inodes", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].free_inodes,
				 "fs", name, "free_inodes", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].avail_inodes,
				 "fs", name, "avail_inodes", NULL);
			add_stat(BYTES, &disk[i].io_size,
				 "fs", name, "io_size", NULL);
			add_stat(BYTES, &disk[i].block_size,
				 "fs", name, "block_size", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].total_blocks,
				 "fs", name, "total_blocks", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].free_blocks,
				 "fs", name, "free_blocks", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].avail_blocks,
				 "fs", name, "avail_blocks", NULL);
			add_stat(UNSIGNED_LONG_LONG, &disk[i].used_blocks,
				 "fs", name, "used_blocks", NULL);

			free(buf);
		}
	}
}

void populate_disk() {
	size_t n, i;
	sg_disk_io_stats *diskio;

	diskio = use_diffs ? sg_get_disk_io_stats_diff(&n)
			   : sg_get_disk_io_stats(&n);
	if (diskio != NULL) {
		for (i = 0; i < n; i++) {
			const char *name = diskio[i].disk_name;
	
			add_stat(STRING, &diskio[i].disk_name,
				 "disk", name, "disk_name", NULL);
			add_stat(BYTES, &diskio[i].read_bytes,
				 "disk", name, "read_bytes", NULL);
			add_stat(BYTES, &diskio[i].write_bytes,
				 "disk", name, "write_bytes", NULL);
			add_stat(TIME_T, &diskio[i].systime,
				 "disk", name, "systime", NULL);
		}
	}
}

void populate_proc() {
	/* FIXME expose individual process info too */
	sg_process_count *proc = sg_get_process_count();

	if (proc != NULL) {
		add_stat(UNSIGNED_LONG_LONG, &proc->total, "proc", "total", NULL);
		add_stat(UNSIGNED_LONG_LONG, &proc->running, "proc", "running", NULL);
		add_stat(UNSIGNED_LONG_LONG, &proc->sleeping, "proc", "sleeping", NULL);
		add_stat(UNSIGNED_LONG_LONG, &proc->stopped, "proc", "stopped", NULL);
		add_stat(UNSIGNED_LONG_LONG, &proc->zombie, "proc", "zombie", NULL);
	}
        else {
                char *errbuf;
                sg_error_details errdet;
                if( SG_ERROR_NONE != sg_get_error_details(&errdet) )
                        return;
                if( NULL == sg_strperror( &errbuf, &errdet ) )
                        return;
                printf( "%s\n", errbuf );
        }
}

void populate_net() {
	size_t num_io, num_iface, i;
	sg_network_io_stats *io;
	sg_network_iface_stats *iface;

	io = use_diffs ? sg_get_network_io_stats_diff(&num_io)
		       : sg_get_network_io_stats(&num_io);
	if (io != NULL) {
		for (i = 0; i < num_io; i++) {
			const char *name = io[i].interface_name;
	
			add_stat(STRING, &io[i].interface_name,
				 "net", name, "interface_name", NULL);
			add_stat(BYTES, &io[i].tx,
				 "net", name, "tx", NULL);
			add_stat(BYTES, &io[i].rx,
				 "net", name, "rx", NULL);
			add_stat(UNSIGNED_LONG_LONG, &io[i].ipackets,
				 "net", name, "ipackets", NULL);
			add_stat(UNSIGNED_LONG_LONG, &io[i].opackets,
				 "net", name, "opackets", NULL);
			add_stat(UNSIGNED_LONG_LONG, &io[i].ierrors,
				 "net", name, "ierrors", NULL);
			add_stat(UNSIGNED_LONG_LONG, &io[i].oerrors,
				 "net", name, "oerrors", NULL);
			add_stat(UNSIGNED_LONG_LONG, &io[i].collisions,
				 "net", name, "collisions", NULL);
			add_stat(TIME_T, &io[i].systime,
				 "net", name, "systime", NULL);
		}
	}

	iface = sg_get_network_iface_stats(&num_iface);
	if (iface != NULL) {
		for (i = 0; i < num_iface; i++) {
			const char *name = iface[i].interface_name;
			int had_io = 0;
			size_t j;

			/* If there wasn't a corresponding io stat,
			   add interface_name from here. */
			if (io != NULL) {
				for (j = 0; j < num_io; j++) {
					if (strcmp(io[j].interface_name,
					           name) == 0) {
						had_io = 1;
						break;
					}
				}
			}
			if (!had_io) {
				add_stat(STRING, &iface[i].interface_name,
				 	"net", name, "interface_name", NULL);
			}

			add_stat(INT, &iface[i].speed,
				 "net", name, "speed", NULL);
			add_stat(BOOL, &iface[i].up,
				 "net", name, "up", NULL);
			add_stat(DUPLEX, &iface[i].duplex,
				 "net", name, "duplex", NULL);
		}
	}
}

void populate_page() {
	sg_page_stats *page;

	page = use_diffs ? sg_get_page_stats_diff() : sg_get_page_stats();
	if (page != NULL) {
		add_stat(UNSIGNED_LONG_LONG, &page->pages_pagein, "page", "in", NULL);
		add_stat(UNSIGNED_LONG_LONG, &page->pages_pageout, "page", "out", NULL);
		add_stat(TIME_T, &page->systime, "page", "systime", NULL);
	}
}

typedef struct {
	const char *name;
	void (*populate)();
	int interesting;
} toplevel;
toplevel toplevels[] = {
	{"const.", populate_const, 0},
	{"cpu.", populate_cpu, 0},
	{"mem.", populate_mem, 0},
	{"load.", populate_load, 0},
	{"user.", populate_user, 0},
	{"swap.", populate_swap, 0},
	{"general.", populate_general, 0},
	{"fs.", populate_fs, 0},
	{"disk.", populate_disk, 0},
	{"proc.", populate_proc, 0},
	{"net.", populate_net, 0},
	{"page.", populate_page, 0},
	{NULL, NULL, 0}
};

/* Set the "interesting" flag on the sections that we actually need to
   fetch. */
void select_interesting(int argc, char **argv) {
	toplevel *t;

	if (argc == 0) {
		for (t = &toplevels[0]; t->name != NULL; t++)
			t->interesting = 1;
	} else {
		int i;

		for (i = 0; i < argc; i++) {
			for (t = &toplevels[0]; t->name != NULL; t++) {
				if (strncmp(argv[i], t->name,
					    strlen(t->name)) == 0) {
					t->interesting = 1;
					break;
				}
			}
		}
	}
}

/* Clear and rebuild the stat_items array. */
void get_stats() {
	toplevel *t;

	clear_stats();

	for (t = &toplevels[0]; t->name != NULL; t++) {
		if (t->interesting)
			t->populate();
	}

	if (stat_items != NULL)
		qsort(stat_items, num_stats, sizeof *stat_items, stats_compare);
}

/* Print the value of a stat_item. */
void print_stat_value(const stat_item *s) {
	void *pv = s->stat;
	union {
		double f;
		long l;
		unsigned long ul;
		long long ll;
		unsigned long long ull;
	} v;

	switch (s->type) {
	case LONG_LONG:
#ifdef WIN32 /* Windows printf does not understand %lld, so use %I64d instead */
		printf("%I64d", *(long long *)pv);
#else
		printf("%lld", *(long long *)pv);
#endif
		break;
	case UNSIGNED_LONG_LONG:
#ifdef WIN32 /* Windows printf does not understand %llu, so use %I64u instead */
		printf("%I64u", *(unsigned long long *)pv);
#else
		printf("%llu", *(unsigned long long *)pv);
#endif
		break;
	case BYTES:
		v.ull = *(unsigned long long *)pv;
		if (bytes_scale_factor != 0) {
			v.ull /= bytes_scale_factor;
		}
#ifdef WIN32
		printf("%I64u", v.ull);
#else
		printf("%llu", v.ull);
#endif
		break;
	case TIME_T:
		/* FIXME option for formatted time? */
		v.l = *(time_t *)pv;
		printf("%ld", v.l);
		break;
	case FLOAT:
	case DOUBLE:
		if (s->type == FLOAT) {
			v.f = *(float *)pv;
		}
		else {
			v.f = *(double *)pv;
		}
		if (float_scale_factor != 0) {
			printf("%ld", (long)(float_scale_factor * v.f));
		}
		else {
			printf("%f", v.f);
		}
		break;
	case STRING:
		/* FIXME escaping? */
		printf("%s", *(char **)pv);
		break;
	case INT:
		printf("%d", *(int *)pv);
		break;
	case UNSIGNED:
		printf("%u", *(unsigned *)pv);
		break;
	case BOOL:
		printf("%s", *(int *)pv ? "true" : "false");
		break;
	case DUPLEX:
		switch (*(sg_iface_duplex *) pv) {
		case SG_IFACE_DUPLEX_FULL:
			printf("full");
			break;
		case SG_IFACE_DUPLEX_HALF:
			printf("half");
			break;
		default:
			printf("unknown");
			break;
		}
		break;
	}
}

/* Print the name and value of a stat. */
void print_stat(const stat_item *s) {
	switch (display_mode) {
	case DISPLAY_LINUX:
		printf("%s = ", s->name);
		break;
	case DISPLAY_BSD:
		printf("%s: ", s->name);
		break;
	case DISPLAY_MRTG:
	case DISPLAY_PLAIN:
		break;
	}
	print_stat_value(s);
	printf("\n");
}

/* Print stats as specified on the provided command line. */
void print_stats(int argc, char **argv) {
	int i;

	if (argc == optind) {
		/* Print all stats. */
		for (i = 0; i < num_stats; i++)
			print_stat(&stat_items[i]);
	} else {
		/* Print selected stats. */
		for (i = optind; i < argc; i++) {
			char *name = argv[i];
			stat_item key;
			const stat_item *s, *end;
			int (*compare)(const void *, const void *);

			key.name = name;
			if (name[strlen(name) - 1] == '.')
				compare = stats_compare_prefix;
			else
				compare = stats_compare;

			if (stat_items == NULL) {
				s = NULL;
			} else {
				s = (const stat_item *)bsearch(&key, stat_items,
							  num_stats,
							  sizeof *stat_items,
							  compare);
			}

			if (s == NULL) {
				printf("Unknown stat %s\n", name);
				continue;
			}

			/* Find the range of stats the user wanted. */
			for (; s >= stat_items; s--) {
				if (compare(&key, s) != 0)
					break;
			}
			s++;
			for (end = s; end < &stat_items[num_stats]; end++) {
				if (compare(&key, end) != 0)
					break;
			}

			/* And print them. */
			for (; s < end; s++) {
				print_stat(s);
			}
		}
	}
}

void usage() {
	printf("Usage: statgrab [OPTION]... [STAT]...\n"
	       "Display system statistics.\n"
	       "\n"
	       "If no STATs are given, all will be displayed. Specify 'STAT.' to display all\n"
	       "statistics starting with that prefix.\n"
	       "\n");
	printf("  -l         Linux sysctl-style output (default)\n"
	       "  -b         BSD sysctl-style output\n"
	       "  -m         MRTG-compatible output\n"
	       "  -u         Plain output (only show values)\n"
	       "  -n         Display cumulative stats once (default)\n"
	       "  -s         Display stat differences repeatedly\n"
	       "  -o         Display stat differences once\n"
	       "  -t DELAY   When repeating, wait DELAY seconds between updates (default 1)\n"
	       "  -p         Display CPU usage differences as percentages rather than\n"
	       "             absolute values\n"
	       "  -f FACTOR  Display floating-point values as integers scaled by FACTOR\n"
	       "  -K         Display byte counts in kibibytes\n"
	       "  -M         Display byte counts in mebibytes\n"
	       "  -G         Display byte counts in gibibytes\n"
	       "\n");
	printf("Version %s - report bugs to <%s>.\n",
	       PACKAGE_VERSION, PACKAGE_BUGREPORT);
	exit(1);
}

int main(int argc, char **argv) {
	opterr = 0;
	while (1) {
		int c = getopt(argc, argv, "lbmunsot:pf:KMG");
		if (c == -1)
			break;
		switch (c) {
		case 'l':
			display_mode = DISPLAY_LINUX;
			break;
		case 'b':
			display_mode = DISPLAY_BSD;
			break;
		case 'm':
			display_mode = DISPLAY_MRTG;
			break;
		case 'u':
			display_mode = DISPLAY_PLAIN;
			break;
		case 'n':
			repeat_mode = REPEAT_NONE;
			break;
		case 's':
			repeat_mode = REPEAT_FOREVER;
			break;
		case 'o':
			repeat_mode = REPEAT_ONCE;
			break;
		case 't':
			repeat_time = atoi(optarg);
			break;
		case 'p':
			use_cpu_percent = 1;
			break;
		case 'f':
			float_scale_factor = atol(optarg);
			break;
		case 'K':
			bytes_scale_factor = 1024;
			break;
		case 'M':
			bytes_scale_factor = 1024 * 1024;
			break;
		case 'G':
			bytes_scale_factor = 1024 * 1024 * 1024;
			break;
		default:
			usage();
		}
	}

	if (display_mode == DISPLAY_MRTG) {
		if ((argc - optind) != 2)
			die("mrtg mode: must specify exactly two stats");
		if (repeat_mode == REPEAT_FOREVER)
			die("mrtg mode: cannot repeat display");
	}
/*
	if (use_cpu_percent && repeat_mode == REPEAT_NONE)
		die("CPU percentage usage display requires stat differences");
*/
	if (repeat_mode == REPEAT_NONE)
		use_diffs = 0;
	else
		use_diffs = 1;

	select_interesting(argc - optind, &argv[optind]);

	/* We don't care if sg_init fails, because we can just display
 	   the statistics that can be read as non-root. */
	sg_init(1);
	sg_snapshot();
	if (sg_drop_privileges() != 0)
		die("Failed to drop setuid/setgid privileges");

	switch (repeat_mode) {
	case REPEAT_NONE:
		get_stats();
		print_stats(argc, argv);
		break;
	case REPEAT_ONCE:
		get_stats();
		sleep(repeat_time);
		sg_snapshot();
		get_stats();
		print_stats(argc, argv);
		break;
	case REPEAT_FOREVER:
		while (1) {
			get_stats();
			print_stats(argc, argv);
			printf("\n");
			sleep(repeat_time);
			sg_snapshot();
		}
	}

	if (display_mode == DISPLAY_MRTG) {
		printf("\n");
		printf("statgrab\n");
	}

	sg_shutdown();

	return 0;
}

