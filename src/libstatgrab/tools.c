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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <regex.h>
#ifdef ALLBSD
#include <fcntl.h>
#endif
#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
#include <kvm.h>
#endif
#if defined(NETBSD) || defined(OPENBSD)
#include <uvm/uvm_extern.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#include "tools.h"

#ifdef SOLARIS
#ifdef HAVE_LIBDEVINFO_H
#include <libdevinfo.h>
#endif
#include <kstat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/dkio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <dirent.h>
#endif

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
char *get_svr_from_bsd(char *bsd){
#ifdef HAVE_LIBDEVINFO_H
	mapping_t *map_ptr;
	for(map_ptr = mapping; map_ptr != NULL; map_ptr = map_ptr->next)
		if(!strcmp(map_ptr->bsd, bsd)) return map_ptr->svr;
#endif
	return bsd;
}
#endif

#if defined(SOLARIS) && defined(HAVE_LIBDEVINFO_H)
void add_mapping(char *bsd, char *svr){
	mapping_t *map_ptr;
	mapping_t *map_end_ptr;

	bsd = strdup(bsd);
	svr = strdup(svr);

	if (mapping == NULL){
		mapping = malloc(sizeof(mapping_t));
		if (mapping == NULL) return;
		map_ptr = mapping;
	}else{
		/* See if its already been added */
		for(map_ptr = mapping; map_ptr != NULL; map_ptr = map_ptr->next){
			if( (!strcmp(map_ptr->bsd, bsd)) || (!strcmp(map_ptr->svr, svr)) ){
				return;
			}
			map_end_ptr = map_ptr;
		}

		/* We've reached end of list and not found the entry.. So we need to malloc
		 * new mapping_t
		 */
		map_end_ptr->next = malloc(sizeof(mapping_t));
		if (map_end_ptr->next == NULL) return;
		map_ptr = map_end_ptr->next;
	}

	map_ptr->next = NULL;
	map_ptr->bsd = bsd;
	map_ptr->svr = svr;

	return;
}

char *read_dir(char *disk_path){
	DIR *dirp;
	struct dirent *dp;
	struct stat stbuf;
	char *svr_name;
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
				svr_name = strdup(dp->d_name);
				closedir(dirp);
				return svr_name;
			}
		}
	}
	closedir(dirp);
	return NULL;
}



int get_alias(char *alias){
	char file[MAXPATHLEN];
	di_node_t root_node;
	di_node_t node;
	di_minor_t minor = DI_MINOR_NIL;
	char tmpnode[MAXPATHLEN];
	char *phys_path;
	char *minor_name;
	char *value;
	int instance;
	if ((root_node = di_init("/", DINFOCPYALL)) == DI_NODE_NIL) {
		return 1;
	}
	node = di_drv_first_node(alias, root_node);
	while (node != DI_NODE_NIL) {
		if ((minor = di_minor_next(node, DI_MINOR_NIL)) != DI_MINOR_NIL) {
			instance = di_instance(node);
			phys_path = di_devfs_path(node);
			minor_name = di_minor_name(minor);
			strcpy(tmpnode, alias);
			sprintf(tmpnode, "%s%d", tmpnode, instance);
			strlcpy(file, "/devices", sizeof file);
			strlcat(file, phys_path, sizeof file);
			strlcat(file, ":", sizeof file);
			strlcat(file, minor_name, sizeof file);
			value = read_dir(file);
			if (value != NULL){
				add_mapping(tmpnode, value);
			}
			di_devfs_path_free(phys_path);
			node = di_drv_next_node(node);
		}else{
			node = di_drv_next_node(node);
		}
	}
	di_fini(root_node);
	return 0;
}


#define BIG_ENOUGH 512
int build_mapping(){
	char device_name[BIG_ENOUGH];
	int x;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_io_t kios;

	char driver_list[BIG_ENOUGH][BIG_ENOUGH];
	int list_entries = 0;
	int found;

	if ((kc = kstat_open()) == NULL) {
		return;
	}

	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
		if (!strcmp(ksp->ks_class, "disk")) {
			if(ksp->ks_type != KSTAT_TYPE_IO) continue;
			/* We dont want metadevices appearing as num_diskio */
			if(strcmp(ksp->ks_module, "md")==0) continue;
			if((kstat_read(kc, ksp, &kios))==-1) continue;
			strncpy(device_name, ksp->ks_name, sizeof device_name);
			for(x=0;x<(sizeof device_name);x++){
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
					return 1;
				}
				if( !strncmp(driver_list[x], device_name, BIG_ENOUGH)){
					found = 1;
					break;
				}
			}

			if(!found){
				if((get_alias(device_name)) != 0){
					return 1;
				}
				strncpy(driver_list[x], device_name, BIG_ENOUGH);
				list_entries++;
			}
		}
	}

	return 0;
}

#endif



char *f_read_line(FILE *f, const char *string){
	/* Max line length. 8k should be more than enough */
	static char line[8192];

	while((fgets(line, sizeof(line), f))!=NULL){
		if(strncmp(string, line, strlen(string))==0){
			return line;
		}
	}

	return NULL;
}

char *get_string_match(char *line, regmatch_t *match){
	int len=match->rm_eo - match->rm_so;
	char *match_string=malloc(len+1);

	match_string=strncpy(match_string, line+match->rm_so, len);
	match_string[len]='\0';

	return match_string;
}



#ifndef HAVE_ATOLL
static long long atoll(const char *s) {
	long long value = 0;
	int isneg = 0;

	while (*s == ' ' || *s == '\t') {
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

#ifndef HAVE_STRLCPY
/*      $OpenBSD: strlcpy.c,v 1.8 2003/06/17 21:56:24 millert Exp $     */

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
size_t strlcpy(char *dst, const char *src, size_t siz){
        register char *d = dst;
        register const char *s = src;
        register size_t n = siz;

        /* Copy as many bytes as will fit */
        if (n != 0 && --n != 0) {
                do {
                        if ((*d++ = *s++) == 0)
                                break;
                } while (--n != 0);
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';              /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        return(s - src - 1);    /* count does not include NUL */
}
#endif

#ifndef HAVE_STRLCAT
/*      $OpenBSD: strlcat.c,v 1.11 2003/06/17 21:56:24 millert Exp $    */

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
size_t strlcat(char *dst, const char *src, size_t siz){
        register char *d = dst;
        register const char *s = src;
        register size_t n = siz;
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

long long get_ll_match(char *line, regmatch_t *match){
	char *ptr;
	long long num;

	ptr=line+match->rm_so;
	num=atoll(ptr);

	return num;
}

#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
kvm_t *get_kvm() {
	static kvm_t *kvmd = NULL;

	if (kvmd != NULL) {
		return kvmd;
	}

	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL);
	return kvmd;
}

/* Can't think of a better name for this function */
kvm_t *get_kvm2() {
	static kvm_t *kvmd2 = NULL;

	if (kvmd2 != NULL) {
		return kvmd2;
	}

	kvmd2 = kvm_openfiles(_PATH_DEVNULL, _PATH_DEVNULL, NULL, O_RDONLY, NULL);
	return kvmd2;
}
#endif

#if defined(NETBSD) || defined(OPENBSD)
struct uvmexp *get_uvmexp() {
	int mib[2];
	size_t size;
	static struct uvmexp *uvm = NULL;

	mib[0] = CTL_VM;
	mib[1] = VM_UVMEXP;

	if (sysctl(mib, 2, NULL, &size, NULL, 0) < 0) {
		return NULL;
	}

	uvm = realloc(uvm, size);
	if (uvm == NULL) {
		return NULL;
	}

	if (sysctl(mib, 2, uvm, &size, NULL, 0) < 0) {
		return NULL;
	}

	return uvm;
}
#endif

int statgrab_init(){
#if (defined(FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
	{ 
		kvm_t *kvmd = get_kvm(); 
		if (kvmd == NULL) return 1;
	}
	{ 
		kvm_t *kvmd2 = get_kvm2();
		if (kvmd2 == NULL) return 1;
	}
#endif
#if defined(NETBSD) || defined(OPENBSD)
	{
		/* This should always succeed, but it seems that on some
		 * versions of NetBSD the first call to get_uvmexp will return
		 * a non-filled-in structure; this is a workaround for that.
		 */
		struct uvmexp *uvm = get_uvmexp();
		if (uvm == NULL) return 1;
	}
#endif
#ifdef SOLARIS
	/* On solaris 7, this will fail if you are not root. But, everything
	 * will still work, just no disk mappings. So we will ignore the exit
	 * status of this, and carry on merrily.
	 */
#ifdef HAVE_LIBDEVINFO_H
	build_mapping();
#endif
#endif
	return 0;
}

int statgrab_drop_privileges() {
	if (setegid(getgid()) != 0) return -1;
	if (seteuid(getuid()) != 0) return -1;
	return 0;
}

