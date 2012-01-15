/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2011 i-scream
 * Copyright (C) 2010,2011 Jens Rehsack
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

#include "tools.h"

static void sg_user_stats_item_init(sg_user_stats *d) {
	d->login_name = NULL;
	d->record_id = NULL;
	d->record_id_size = 0;
	d->device = NULL;
	d->hostname = NULL;
	d->login_time = 0;
	d->systime = 0;
}

#if 0
static sg_error sg_user_stats_item_copy(sg_user_stats *d, const sg_user_stats *s) {

	if( SG_ERROR_NONE != sg_update_string(&d->login_name, s->login_name) ||
	    SG_ERROR_NONE != sg_update_mem(&d->record_id, s->record_id, s->record_id_size) ||
	    SG_ERROR_NONE != sg_update_string(&d->device, s->device) ||
	    SG_ERROR_NONE != sg_update_string(&d->hostname, s->hostname) ) {
		RETURN_FROM_PREVIOUS_ERROR( "user", sg_get_error() );
	}

	d->record_id_size = s->record_id_size;
	d->pid = s->pid;
	d->tv = s->tv;

	return SG_ERROR_NONE;
}
#else
#define sg_user_stats_item_copy NULL
#endif

#define sg_user_stats_item_compute_diff NULL
#define sg_user_stats_item_compare NULL

static void sg_user_stats_item_destroy(sg_user_stats *d) {
	free(d->login_name);
	free(d->record_id);
	free(d->device);
	free(d->hostname);
}

VECTOR_INIT_INFO_FULL_INIT(sg_user_stats);

static sg_error
sg_get_user_stats_int(sg_vector **user_stats_vector_ptr){
	size_t num_users = 0;
	sg_user_stats *user_ptr;
	time_t now = time(NULL);

#if defined (WIN32)
	LPWKSTA_USER_INFO_0 buf = NULL;
	LPWKSTA_USER_INFO_0 tmp_buf;
	unsigned long entries_read = 0;
	unsigned long entries_tot = 0;
	unsigned long resumehandle = 0;
	NET_API_STATUS nStatus;
	int i;
	char name[256];

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP if (buf != NULL) NetApiBufferFree(buf);

	do {
		nStatus = NetWkstaUserEnum(NULL, 0, (LPBYTE*)&buf,
				MAX_PREFERRED_LENGTH, &entries_read,
				&entries_tot, &resumehandle);
		if((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
			if((tmp_buf = buf) == NULL)
				continue;

			for( i = 0; i < entries_read; ++i ) {
				/* assert(tmp_buf != NULL); */
				if( tmp_buf == NULL ) {
					sg_set_error(SG_ERROR_PERMISSION, "User list");
                                        ERROR_LOG("user", "Permission denied fetching user details");
					break; /* XXX break and not return? */
				}
				/* It's in unicode. We are not. Convert */
				WideCharToMultiByte(CP_ACP, 0, tmp_buf->wkui0_username, -1, name, sizeof(name), NULL, NULL);

				VECTOR_UPDATE(user_stats_vector_ptr, num_users + 1, user_ptr, sg_user_stats);
				if( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, name ) ) {
					VECTOR_UPDATE_ERROR_CLEANUP
					RETURN_FROM_PREVIOUS_ERROR( "user", sg_get_error() );
				}

				user_ptr[num_users].systime = now;

				++tmp_buf;
				++num_users;
			}
		}
		else {
			RETURN_WITH_SET_ERROR("user", SG_ERROR_PERMISSION, "User enum");
		}

		if (buf != NULL) {
			NetApiBufferFree(buf);
			buf=NULL;
		}
	} while (nStatus == ERROR_MORE_DATA);

	if (buf != NULL)
		NetApiBufferFree(buf);
#elif defined(HAVE_GETUTXENT)
	struct utmpx *ut;

#define UTMP_MUTEX_NAME "utmp"

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP endutxent(); sg_unlock_mutex(UTMP_MUTEX_NAME);

	sg_lock_mutex(UTMP_MUTEX_NAME);
	setutxent();
	while( NULL != (ut = getutxent()) ) {
		if( USER_PROCESS != ut->ut_type )
			continue;

		VECTOR_UPDATE(user_stats_vector_ptr, num_users + 1, user_ptr, sg_user_stats);

		if( ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, ut->ut_user ) ) ||
#if defined(HAVE_UTMPX_HOST)
#if defined(HAVE_UTMPX_SYSLEN)
		    ( SG_ERROR_NONE != sg_lupdate_string( &user_ptr[num_users].hostname, ut->ut_host, ut->ut_syslen + 1 ) ) ||
#else
		    ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].hostname, ut->ut_host ) ) ||
#endif
#endif
		    ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].device, ut->ut_line ) ) ||
		    ( SG_ERROR_NONE != sg_update_mem( (void *)(&user_ptr[num_users].record_id), ut->ut_id, sizeof(ut->ut_id) ) ) ) {
			    VECTOR_UPDATE_ERROR_CLEANUP
			    RETURN_FROM_PREVIOUS_ERROR( "user", sg_get_error() );
		}

		user_ptr[num_users].record_id_size = sizeof(ut->ut_id);
		user_ptr[num_users].pid = ut->ut_pid;
		user_ptr[num_users].login_time = ut->ut_tv.tv_sec;
		user_ptr[num_users].systime = now;

		++num_users;
	}

	endutxent();
	sg_unlock_mutex(UTMP_MUTEX_NAME);
#elif defined(HAVE_GETUTENT)
	struct utmp *ut;

#define UTMP_MUTEX_NAME "utmp"

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP endutent(); sg_unlock_mutex(UTMP_MUTEX_NAME);

	sg_lock_mutex(UTMP_MUTEX_NAME);
	setutent();
	while( NULL != (ut = getutent()) ) {
#ifdef HAVE_UTMP_TYPE
		if( USER_PROCESS != ut->ut_type )
			continue;
#elif defined(HAVE_UTMP_NAME)
		if (entry.ut_name[0] == '\0')
			continue;
#elif defined(HAVE_UTMP_USER)
		if (entry.ut_user[0] == '\0')
			continue;
#endif

		VECTOR_UPDATE(user_stats_vector_ptr, num_users + 1, user_ptr, sg_user_stats);

		if( ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].device, ut->ut_line ) )
#if defined(HAVE_UTMP_USER)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, ut->user ) )
#elif defined(HAVE_UTMP_NAME)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, ut->name ) )
#endif
#if defined(HAVE_UTMP_HOST)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].hostname, ut->ut_host ) )
#endif
#if defined(HAVE_UTMP_ID)
		 || ( SG_ERROR_NONE != sg_update_mem( &user_ptr[num_users].record_id, ut->ut_id, sizeof(ut->ut_id) ) )
#endif
		) {
			    VECTOR_UPDATE_ERROR_CLEANUP
			    RETURN_FROM_PREVIOUS_ERROR( "user", sg_get_error() );
		}

#if defined(HAVE_UTMP_ID)
		user_ptr[num_users].record_id_size = sizeof(ut->ut_id);
#endif
#if defined(HAVE_UTMP_PID)
		user_ptr[num_users].pid = ut->ut_pid;
#endif
		user_ptr[num_users].login_time = ut->ut_tv.tv_sec;
		user_ptr[num_users].systime = now;

		++num_users;
	}

	endutent();
	sg_unlock_mutex(UTMP_MUTEX_NAME);
#elif defined(HAVE_STRUCT_UTMP) && defined(_PATH_UTMP)
	struct utmp entry;
	FILE *f;

	if ((f=fopen(_PATH_UTMP, "r")) == NULL) {
		RETURN_WITH_SET_ERROR_WITH_ERRNO("user", SG_ERROR_OPEN, _PATH_UTMP);
        }

#undef VECTOR_UPDATE_ERROR_CLEANUP
#define VECTOR_UPDATE_ERROR_CLEANUP fclose(f);

	while((fread(&entry, sizeof(entry),1,f)) != 0){
#ifdef HAVE_UTMP_TYPE
		if( USER_PROCESS != ut->ut_type )
			continue;
#elif defined(HAVE_UTMP_NAME)
		if (entry.ut_name[0] == '\0')
			continue;
#elif defined(HAVE_UTMP_USER)
		if (entry.ut_user[0] == '\0')
			continue;
#endif

		VECTOR_UPDATE(user_stats_vector_ptr, num_users + 1, user_ptr, sg_user_stats);

		if( ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].device, entry.ut_line ) )
#if defined(HAVE_UTMP_USER)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, entry.ut_user ) )
#elif defined(HAVE_UTMP_NAME)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].login_name, entry.ut_name ) )
#endif
#if defined(HAVE_UTMP_HOST)
		 || ( SG_ERROR_NONE != sg_update_string( &user_ptr[num_users].hostname, entry.ut_host ) )
#endif
#if defined(HAVE_UTMP_ID)
		 || ( SG_ERROR_NONE != sg_update_mem( &user_ptr[num_users].record_id, entry.ut_id, sizeof(entry.ut_id) ) )
#endif
		) {
			    VECTOR_UPDATE_ERROR_CLEANUP
			    RETURN_FROM_PREVIOUS_ERROR( "user", sg_get_error() );
		}

#if defined(HAVE_UTMP_ID)
		user_ptr[num_users].record_id_size = sizeof(entry.ut_id);
#endif
#if defined(HAVE_UTMP_PID)
		user_ptr[num_users].pid = entry.ut_pid;
#endif
#if defined(HAVE_UTMP_TIME)
		user_ptr[num_users].login_time = entry.ut_time;
#endif
		user_ptr[num_users].systime = now;

		++num_users;
	}

	fclose(f);
#else
	RETURN_WITH_SET_ERROR("user", SG_ERROR_UNSUPPORTED, OS_TYPE);
#endif

	return SG_ERROR_NONE;
}

/*
 * setup code
 */

#define SG_USER_STATS_NOW_IDX	0
#define SG_USER_MAX_IDX		1

#if defined(UTMP_MUTEX_NAME)
EASY_COMP_SETUP(user,SG_USER_MAX_IDX,UTMP_MUTEX_NAME,NULL);
#else
EASY_COMP_SETUP(user,SG_USER_MAX_IDX,NULL);
#endif

MULTI_COMP_ACCESS(sg_get_user_stats,user,user,SG_USER_STATS_NOW_IDX)
