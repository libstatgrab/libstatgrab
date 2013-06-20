/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2010-2013 i-scream
 * Copyright (C) 2010-2013 Jens Rehsack
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 * $Id$
 */
#ifndef __SG_TESTLIB_H_INCLUDED__
#define __SG_TESTLIB_H_INCLUDED__

/* log.c */
void log_init(int argc, char **argv);

/* routines.c */
typedef void * (*statgrab_stat_fn)(size_t *entries);

struct statgrab_testfuncs
{
	char const *stat_name;
	char const * full_name;
	statgrab_stat_fn full_fn;
	char const * diff_name;
	statgrab_stat_fn diff_fn;
	unsigned needed;
	unsigned succeeded;
	unsigned done;
};

struct statgrab_testfuncs *get_testable_functions(size_t *entries);
size_t funcnames_to_indices(const char *name_list, size_t **indices, int full);
void print_testable_functions(int full);
size_t report_testable_functions(int full);

void mark_func(size_t func_index);
int run_func(size_t func_index, int full);
void done_func(size_t func_index, int succeeded);

/* err.c */
int sg_warn(const char *prefix);
void sg_die(const char *prefix, int exit_code);
void die(int error, const char *fmt, ...);

/* opt.c */
enum opt_type {
	opt_flag = 0,
	opt_bool,
	opt_signed,
	opt_unsigned,
	opt_str
};

struct opt_def {
	char optchr;
	enum opt_type opttype;
	union {
		long int s;
		unsigned long int u;
		bool b;
		char *str;
	} optarg;
};

int get_params(struct opt_def *opt_defs, int argc, char **argv);

#endif /* ?__SG_TESTLIB_H_INCLUDED__ */
