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

ssize_t sys_page_size = 0;

#if defined(SOLARIS) && defined(HAVE_LIBDEVINFO_H)
struct map{
	char *bsd;
	char *svr;

	struct map *next;
};
typedef struct map mapping_t;

static mapping_t *mapping = NULL;
#endif

#ifdef SOLARIS
const char *sg_get_svr_from_bsd(const char *bsd){
#ifdef HAVE_LIBDEVINFO_H
	mapping_t *map_ptr;
	for(map_ptr = mapping; map_ptr != NULL; map_ptr = map_ptr->next)
		if(!strcmp(map_ptr->bsd, bsd))
			return map_ptr->svr;
#endif
	return bsd;
}
#endif

#if defined(SOLARIS) && defined(HAVE_LIBDEVINFO_H)
static void
add_mapping(char *bsd, char *svr){
	mapping_t *map_ptr = NULL; /* get rid of wrong "might uninitialized" warning */

	if( mapping == NULL ) {
		mapping = sg_malloc(sizeof(mapping_t));
		if (mapping == NULL)
			return;
		map_ptr = mapping;
	}
	else {
		mapping_t *map_end_ptr = map_ptr; /* get rid of wrong "might uninitialized" warning */
		/* See if its already been added */
		for(map_ptr = mapping; map_ptr != NULL; map_ptr = map_ptr->next){
			if( (0 == strcmp(map_ptr->bsd, bsd)) || (0 == strcmp(map_ptr->svr, svr)) ){
				return;
			}
			map_end_ptr = map_ptr;
		}

		/* We've reached end of list and not found the entry.. So we need to malloc
		 * new mapping_t
		 */
		map_end_ptr->next = sg_malloc(sizeof(mapping_t));
		if (map_end_ptr->next == NULL)
			return;
		map_ptr = map_end_ptr->next;
	}

	map_ptr->next = NULL;
	map_ptr->bsd = NULL;
	map_ptr->svr = NULL;
	if( ( SG_ERROR_NONE != sg_update_string(&map_ptr->bsd, bsd) ) ||
	    ( SG_ERROR_NONE != sg_update_string(&map_ptr->svr, svr) ) ) {
		char *buf = NULL;
		ERROR_LOG_FMT("tools", "add_mapping(): %s", sg_strperror(&buf, NULL));
		free(buf);
		return;
	}

	return;
}


static char *
read_dir(char *disk_path){
	DIR *dirp;
	struct dirent *dp;
	struct stat stbuf;
	char *svr_name = NULL;
	char current_dir[MAXPATHLEN];
	char file_name[MAXPATHLEN];
	char temp_name[MAXPATHLEN];
	char dir_dname[MAXPATHLEN];
	char *dsk_dir;
	int x;

	dsk_dir = "/dev/osa/dev/dsk";
	strncpy(current_dir, dsk_dir, sizeof current_dir);
	if ((dirp = opendir(current_dir)) == NULL){
		dsk_dir = "/dev/dsk";
		snprintf(current_dir, sizeof current_dir, "%s", dsk_dir);
		if ((dirp = opendir(current_dir)) == NULL){
			SET_ERROR_WITH_ERRNO( "tools", SG_ERROR_OPENDIR, "open_dir(dsk_dir = %s) in read_dir(disk_path = %s)", dsk_dir ? dsk_dir : "<NULL>", disk_path ? disk_path : "<NULL>" );
			return NULL;
		}
	}

	while ((dp = readdir(dirp)) != NULL){
		snprintf(temp_name, sizeof temp_name, "../..%s", disk_path);
		snprintf(dir_dname, sizeof dir_dname, "%s/%s", dsk_dir, dp->d_name);
		stat(dir_dname,&stbuf);

		if (S_ISBLK(stbuf.st_mode)){
			x = readlink(dir_dname, file_name, sizeof(file_name));
			file_name[x] = '\0';
			if (strcmp(file_name, temp_name) == 0) {
				if (SG_ERROR_NONE != sg_update_string(&svr_name, dp->d_name) ) {
					char *buf = NULL;
					ERROR_LOG_FMT("tools", "read_dir(): %s", sg_strperror(&buf, NULL));
					free(buf);
					return NULL;
				}

				closedir(dirp);
				return svr_name;
			}
		}
	}
	closedir(dirp);
	return NULL;
}


static int
get_alias(char *alias){
	char file[MAXPATHLEN];
	di_node_t root_node;
	di_node_t node;
	di_minor_t minor = DI_MINOR_NIL;
	char tmpnode[MAXPATHLEN + 1];
	char *phys_path;
	char *minor_name;
	char *value;
	int instance;
	if ((root_node = di_init("/", DINFOCPYALL)) == DI_NODE_NIL) {
		return -1;
	}
	node = di_drv_first_node(alias, root_node);
	while (node != DI_NODE_NIL) {
		if ((minor = di_minor_next(node, DI_MINOR_NIL)) != DI_MINOR_NIL) {
			instance = di_instance(node);
			phys_path = di_devfs_path(node);
			minor_name = di_minor_name(minor);
			/* sg_strlcpy(tmpnode, alias, MAXPATHLEN); */
			snprintf(tmpnode, sizeof(tmpnode), "%s%d", alias, instance);
			sg_strlcpy(file, "/devices", sizeof file);
			sg_strlcat(file, phys_path, sizeof file);
			sg_strlcat(file, ":", sizeof file);
			sg_strlcat(file, minor_name, sizeof file);
			value = read_dir(file);
			if (value != NULL) {
				add_mapping(tmpnode, value);
			}
			di_devfs_path_free(phys_path);
			node = di_drv_next_node(node);
		}
		else {
			node = di_drv_next_node(node);
		}
	}
	di_fini(root_node);
	return 0;
}


#define BIG_ENOUGH 512
static int
build_mapping(void) {
	char device_name[BIG_ENOUGH];
	int x;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_io_t kios;

	char driver_list[BIG_ENOUGH][BIG_ENOUGH];
	int list_entries = 0;
	int found;

	if ((kc = kstat_open()) == NULL) {
		return -1;
	}

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (!strcmp(ksp->ks_class, "disk")) {
			if(ksp->ks_type != KSTAT_TYPE_IO) continue;
			/* We dont want metadevices appearing as num_diskio */
			if(strcmp(ksp->ks_module, "md")==0) continue;
			if((kstat_read(kc, ksp, &kios))==-1) continue;
			strncpy(device_name, ksp->ks_name, sizeof device_name);
			for(x=0;x<(int)(sizeof device_name);x++){
				if( isdigit((int)device_name[x]) ) break;
			}
			if(x == sizeof device_name) x--;
			device_name[x] = '\0';

			/* Check if we've not already looked it up */
			found = 0;
			for(x=0;x<list_entries;x++){
				if (x>=BIG_ENOUGH){
					/* We've got bigger than we thought was massive */
					/* If we hit this.. Make big enough bigger */
					kstat_close(kc);
					return -1;
				}
				if( !strncmp(driver_list[x], device_name, BIG_ENOUGH)){
					found = 1;
					break;
				}
			}

			if(!found){
				if((get_alias(device_name)) != 0){
					kstat_close(kc);
					return -1;
				}
				strncpy(driver_list[x], device_name, BIG_ENOUGH);
				list_entries++;
			}
		}
	}

	kstat_close(kc);

	return 0;
}

#endif

#if defined(LINUX) || defined(CYGWIN)
char *
sg_f_read_line( FILE *f, char *linebuf, size_t buf_size, const char *string ) {
	assert(linebuf);
	if( !linebuf ) {
		SET_ERROR("tools", SG_ERROR_INVALID_ARGUMENT, "sg_f_read_line(linebuf = %p", linebuf);
		return NULL;
	}

	while( ( fgets( linebuf, buf_size, f ) ) != NULL ) {
		if(string && (strncmp(string, linebuf, strlen(string))==0)){
			return linebuf;
		}
	}

	if( !feof( f ) ) {
		SET_ERROR_WITH_ERRNO("tools", SG_ERROR_PARSE, "sg_f_read_line(string = %s)", string ? string : "<NULL>");
	}

	return NULL;
}

char *
sg_get_string_match(char *line, regmatch_t *match) {

	regoff_t len;
	char *match_string;

	assert(line);
	assert(match);

	if( !line || !match ) {
		SET_ERROR("tools", SG_ERROR_INVALID_ARGUMENT, "sg_get_string_match(line = %p, match = %p)", line, match);
		return NULL;
	}

	len = match->rm_eo - match->rm_so;
	match_string = strndup(line + match->rm_so, len);
	if( NULL == match_string ) {
		SET_ERROR_WITH_ERRNO("tools", SG_ERROR_MALLOC, "sg_get_string_match: couldn't strndup()");
	}

	return match_string;
}

/* Cygwin (without a recent newlib) doesn't have atoll */
#ifndef HAVE_ATOLL
static long long
atoll(const char *s) {
	long long value = 0;
	int isneg = 0;

	while ( ispace(*s) ) {
		s++;
	}
	if (*s == '-') {
		isneg = 1;
		s++;
	}
	while (*s >= '0' && *s <= '9') {
		value = (10 * value) + (*s - '0');
		s++;
	}
	return (isneg ? -value : value);
}
#endif

long long
sg_get_ll_match(char *line, regmatch_t *match){
	char *ptr;
	long long num;

	ptr = line + match->rm_so;
	num = atoll(ptr);

	return num;
}
#endif

#ifndef HAVE_STRLCPY
/*	$OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
sg_strlcpy(char *dst, const char *src, size_t siz){
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';	      /* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);    /* count does not include NUL */
}
#endif

#ifndef HAVE_STRLCAT
/*	$OpenBSD: strlcat.c,v 1.13 2005/08/08 08:05:37 espie Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
sg_strlcat(char *dst, const char *src, size_t siz){
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));       /* count does not include NUL */
}
#endif

sg_error
sg_update_string(char **dest, const char *src) {
	char *new;

	if (src == NULL) {
		/* We're being told to set it to NULL. */
		free(*dest);
		*dest = NULL;
		return SG_ERROR_NONE;
	}

	new = sg_realloc(*dest, strlen(src) + 1);
	if (new == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "tools", sg_get_error() );
	}

	sg_strlcpy(new, src, strlen(src) + 1);
	*dest = new;
	return SG_ERROR_NONE;
}

sg_error
sg_lupdate_string(char **dest, const char *src, size_t maxlen) {
	char *new;
	size_t srclen, newlen;

	if (src == NULL) {
		/* We're being told to set it to NULL. */
		free(*dest);
		*dest = NULL;
		return SG_ERROR_NONE;
	}

	srclen = strlen(src) + 1;
	newlen = MIN(srclen,maxlen);
	new = sg_realloc(*dest, newlen);
	if (new == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "tools", sg_get_error() );
	}

	sg_strlcpy(new, src, newlen);
	*dest = new;
	return SG_ERROR_NONE;
}

sg_error
sg_update_mem(void **dest, const void *src, size_t len) {
	char *new;

	if (src == NULL) {
		/* We're being told to set it to NULL. */
		free(*dest);
		*dest = NULL;
		return SG_ERROR_NONE;
	}

	new = sg_realloc(*dest, len);
	if (new == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "tools", sg_get_error() );
	}

	memcpy(new, src, len);
	*dest = new;
	return SG_ERROR_NONE;
}

/* join two strings together */
sg_error
sg_concat_string(char **dest, const char *src) {
	char *new;
	size_t len = 0;

	if( NULL == dest ) {
		RETURN_WITH_SET_ERROR("tools", SG_ERROR_INVALID_ARGUMENT, "dest");
	}

	if(*dest)
		len += strlen(*dest);
	if(src)
		len += strlen(src);
	++len;

	new = sg_realloc(*dest, len);
	if (new == NULL) {
		RETURN_FROM_PREVIOUS_ERROR( "tools", sg_get_error() );
	}

	*dest = new;
	sg_strlcat(*dest, src, len);
	return SG_ERROR_NONE;
}

sg_error
sg_init(int ignore_init_errors) {
	sg_error rc = sg_comp_init(ignore_init_errors);

#if defined(SOLARIS) && defined(HAVE_LIBDEVINFO_H)
	/* On solaris 7, this will fail if you are not root. But, everything
	 * will still work, just no disk mappings. So we will ignore the exit
	 * status of this, and carry on merrily.
	 */
	build_mapping();
#endif

	return rc;

#if 0
#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
	if (sg_get_kvm() == NULL) {
		return -1;
	}
	if (sg_get_kvm2() == NULL) {
		return -1;
	}
#endif
	return 0;
#endif
}

sg_error sg_shutdown() {
	return sg_comp_destroy();
}

sg_error sg_snapshot() {
#ifdef WIN32
	return sg_win32_snapshot();
#else
	return SG_ERROR_NONE;
#endif
}

sg_error sg_drop_privileges() {
#ifndef WIN32
#ifdef HAVE_SETEGID
	if (setegid(getgid()) != 0) {
#elif defined(HAVE_SETRESGID)
	if (setresgid(getgid(), getgid(), getgid()) != 0) {
#else
	{
#endif
		RETURN_WITH_SET_ERROR_WITH_ERRNO("tools", SG_ERROR_SETEGID, NULL);
	}
#ifdef HAVE_SETEUID
	if (seteuid(getuid()) != 0) {
#elif defined(HAVE_SETRESUID)
	if (setresuid(getuid(), getuid(), getuid()) != 0) {
#else
	{
#endif
		RETURN_WITH_SET_ERROR_WITH_ERRNO("tools", SG_ERROR_SETEUID, NULL);
	}
#endif /* WIN32 */
	return SG_ERROR_NONE;
}

void *
sg_realloc(void *ptr, size_t size) {
	if( 0 == size ) {
		free(ptr);
		return NULL;
	}
	else {
		void *tmp = realloc(ptr, size);
		if(tmp == NULL) {
			SET_ERROR_WITH_ERRNO("tools", SG_ERROR_MALLOC, "sg_realloc: couldn't realloc(to %lu bytes)", (unsigned long)size);
		}
		return tmp;
	}
}

#if !defined(HAVE_FLOCK) && defined(HAVE_FCNTL) && defined(HAVE_DECL_F_SETLK)
int
flock(int fd, int op)
{
    int try_lock = (0 != (op & LOCK_NB));
    struct flock fl;

    op &= ~LOCK_NB;
    switch(op)
    {
    case LOCK_SH:
	fl.l_type = F_RDLCK;
	break;

    case LOCK_EX:
	fl.l_type = F_WRLCK;
	break;

    case LOCK_UN:
	fl.l_type = F_UNLCK;
	break;

    default:
	errno = EINVAL;
	return -1;
    }

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_whence = SEEK_SET;

    return fcntl( fd, try_lock ? F_SETLK : F_SETLKW, &fl );
}
#endif

#if (defined(HAVE_GETMNTENT) || defined(HAVE_GETMNTENT_R) )
#  if !defined(HAVE_DECL_SETMNTENT) || !defined(HAVE_DECL_ENDMNTENT)
FILE *
setmntent(const char *fn, const char *type) {
	FILE *f;

	if( ( f = fopen(fn, type) ) == NULL ) {
		SET_ERROR_WITH_ERRNO("tools", SG_ERROR_OPEN, "setmntent: fopen(%s, %s)", fn, type);
		return NULL;
	}

	if( flock( fileno( f ), LOCK_SH ) != 0 )
	{
		SET_ERROR_WITH_ERRNO("tools", SG_ERROR_OPEN, "setmntent: flock(%s)", fn);
		fclose(f);
		return NULL;
	}

	return f;
}

int
endmntent(FILE *f) {
	return fclose(f);
}
#  endif
#endif
