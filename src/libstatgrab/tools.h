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
#include <regex.h>
#ifdef ALLBSD
#include <kvm.h>
#endif
#ifdef NETBSD
#include <uvm/uvm_extern.h>
#endif

char *f_read_line(FILE *f, const char *string);

char *get_string_match(char *line, regmatch_t *match);

#ifdef HAVE_ATOLL
long long get_ll_match(char *line, regmatch_t *match);
#endif

#ifdef ALLBSD
kvm_t *get_kvm(void);
#endif

#ifdef NETBSD
struct uvmexp *get_uvmexp(void);
#endif

