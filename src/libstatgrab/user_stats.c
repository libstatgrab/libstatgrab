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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "statgrab.h"
#include "vector.h"
#ifdef ALLBSD
#include <sys/types.h>
#endif
#if defined(NETBSD) || defined(OPENBSD)
#include <limits.h>
#endif
#ifdef OPENBSD
#include <sys/param.h>
#endif
#include <utmp.h>
#ifdef CYGWIN
#include <sys/unistd.h>
#endif

user_stat_t *get_user_stats(){
	int num_users = 0, pos = 0, new_pos;
	VECTOR_DECLARE_STATIC(name_list, char, 128, NULL, NULL);
	static user_stat_t user_stats;
#if defined(SOLARIS) || defined(LINUX) || defined(CYGWIN)
	struct utmp *entry;
#endif
#ifdef ALLBSD
	struct utmp entry;
        FILE *f;
#endif

#if defined(SOLARIS) || defined(LINUX) || defined(CYGWIN)
	setutent();
	while((entry=getutent()) != NULL) {
		if (entry->ut_type != USER_PROCESS) continue;

		new_pos = pos + strlen(entry->ut_user) + 1;
		if (VECTOR_RESIZE(name_list, new_pos) < 0) {
			return NULL;
		}

		strcpy(name_list + pos, entry->ut_user);
		name_list[new_pos - 1] = ' ';
		pos = new_pos;
		num_users++;
	}
	endutent();
#endif
#ifdef ALLBSD
	if ((f=fopen(_PATH_UTMP, "r")) == NULL){
		return NULL;
	}
	while((fread(&entry, sizeof(entry),1,f)) != 0){
		if (entry.ut_name[0] == '\0') continue;

		new_pos = pos + strlen(entry.ut_name) + 1;
		if (VECTOR_RESIZE(name_list, new_pos) < 0) {
			return NULL;
		}

		strcpy(name_list + pos, entry.ut_name);
		name_list[new_pos - 1] = ' ';
		pos = new_pos;
		num_users++;
	}
	fclose(f);
#endif

	/* Remove the extra space at the end, and append a \0. */
	if (num_users != 0) {
		pos--;
	}
	VECTOR_RESIZE(name_list, pos + 1);
	name_list[pos] = '\0';

	user_stats.num_entries = num_users;
	user_stats.name_list = name_list;
	return &user_stats;
}
