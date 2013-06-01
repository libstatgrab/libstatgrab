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
typedef void * (*statgrab_single_fn)(void);
typedef void * (*statgrab_multi_fn)(size_t *entries);

union statgrab_testfunc_addr {
	statgrab_single_fn uniq;
	statgrab_multi_fn multi;
};

struct statgrab_testfuncs
{
	char *fn_name;
	int need_entries_parm;
	union statgrab_testfunc_addr fn;
	int needed;
	int done;
};

struct statgrab_testfuncs *get_testable_functions(size_t *entries);
size_t funcnames_to_indices(const char *name_list, size_t **indices);
void mark_func(size_t func_index);
void run_func(size_t func_index);
void done_func(size_t func_index);

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
