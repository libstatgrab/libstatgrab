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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "statgrab.h"
#ifdef SOLARIS
#include <sys/loadavg.h>
#endif

load_stat_t *get_load_stats(){

	static load_stat_t load_stat;

	double loadav[3];

#ifdef CYGWIN
	return NULL;
#else

	getloadavg(loadav,3);

	load_stat.min1=loadav[0];
	load_stat.min5=loadav[1];
	load_stat.min15=loadav[2];

	return &load_stat;
#endif
}
