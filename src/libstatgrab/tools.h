/*
 * i-scream central monitoring system
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
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

#include <stdio.h>
#include <regex.h>
#if defined(FREEBSD) || defined(DFBSD)
#include <kvm.h>
#endif
#ifdef NETBSD
#include <uvm/uvm_extern.h>
#endif

#ifndef HAVE_STRLCPY
size_t strlcat(char *dst, const char *src, size_t siz);
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

char *update_string(char **dest, const char *src);

long long get_ll_match(char *line, regmatch_t *match);

char *f_read_line(FILE *f, const char *string);

char *get_string_match(char *line, regmatch_t *match);

#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
kvm_t *get_kvm(void);
kvm_t *get_kvm2(void);
#endif

#if defined(NETBSD) || defined(OPENBSD)
struct uvmexp *get_uvmexp(void);
#endif

