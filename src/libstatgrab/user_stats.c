/* 
 * i-scream central monitoring system
 * http://www.i-scream.org.uk
 * Copyright (C) 2000-2002 i-scream
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utmp.h>
#include "statgrab.h"

#define START_VAL (5*(1+MAX_LOGIN_NAME_SIZE))

user_stat_t *get_user_stats(){
	int num_users=0;

	static user_stat_t user_stats;
	static int size_of_namelist=-1;
	char *tmp;
	struct utmp *entry;

	/* First case call */
	if (size_of_namelist==-1){
		user_stats.name_list=malloc(START_VAL);
		if(user_stats.name_list==NULL){
			return NULL;
		}
		size_of_namelist=START_VAL;
	}	

	/* Essentially blank the list, or give it a inital starting string */
	strcpy(user_stats.name_list, "");
	setutent();
	while((entry=getutent()) != NULL) {
		if(entry->ut_type==USER_PROCESS) {
			if((strlen(user_stats.name_list)+MAX_LOGIN_NAME_SIZE+2) > size_of_namelist){
				tmp=user_stats.name_list;
				user_stats.name_list=realloc(user_stats.name_list, 1+(size_of_namelist*2));
				if(user_stats.name_list==NULL){
					user_stats.name_list=tmp;
					return NULL;
				}
				size_of_namelist=1+(size_of_namelist*2);
			}

			strncat(user_stats.name_list, entry->ut_user, MAX_LOGIN_NAME_SIZE);
			strcat(user_stats.name_list, " ");
			num_users++;
		}
	}
	endutent();

	/* We want to remove the last " " */
	if(num_users!=0){
		tmp=strrchr(user_stats.name_list, ' ');
		if(tmp!=NULL){
			*tmp='\0';
			user_stats.num_entries=num_users;
		}
	}

	return &user_stats;

}
