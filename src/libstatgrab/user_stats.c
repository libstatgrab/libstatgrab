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
#include "statgrab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef FREEBSD
#include <sys/types.h>
#endif
#include <utmp.h>

#define START_VAL 5

user_stat_t *get_user_stats(){
	int num_users=0;

	static user_stat_t user_stat;
	static int watermark=-1;
	struct utmp *entry;

	name *name_ptr;

	/* First case call */
	if (watermark==-1){
		user_stat.name_list=malloc(START_VAL * sizeof *user_stat.name_list);
		if(user_stat.name_list==NULL){
			return NULL;
		}
		watermark=START_VAL;
	}	

	setutent();
	while((entry=getutent()) != NULL) {
		if(entry->ut_type==USER_PROCESS) {
			if(num_users>watermark-1){
				name_ptr=user_stat.name_list;
				if((user_stat.name_list=realloc(user_stat.name_list, (watermark*2* sizeof *user_stat.name_list)))==NULL){
					user_stat.name_list=name_ptr;
					return NULL;
				}
				watermark=watermark*2;
			}

			strncpy(user_stat.name_list[num_users], entry->ut_user, MAX_LOGIN_NAME_SIZE);
			/* NULL terminate just in case , the size of user_stat.name_list is MAX_LOGIN_NAME_SIZE+1 */

			user_stat.name_list[num_users][MAX_LOGIN_NAME_SIZE]='\0';
				
			num_users++;
		}
	}
	endutent();
	user_stat.num_entries=num_users;

	return &user_stat;

}
