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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#ifdef ALLBSD
#include <fcntl.h>
#include <kvm.h>
#endif
#ifdef NETBSD
#include <uvm/uvm_extern.h>
#endif

#include "tools.h"

#ifdef SOLARIS
#include <libdevinfo.h>
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

#ifdef SOLARIS
struct map{
	char *bsd;
	char *svr;

	struct map *next;
};
typedef struct map mapping_t;

static mapping_t *mapping = NULL; 

char *get_svr_from_bsd(char *bsd){
	mapping_t *map_ptr;
	for(map_ptr = mapping; map_ptr != NULL; map_ptr = map_ptr->next)
		if(!strcmp(map_ptr->bsd, bsd)) return map_ptr->svr;

	return bsd;
}

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
		fprintf(stderr, "di_init() failed\n");
		exit(1);
	}
	node = di_drv_first_node(alias, root_node);
	while (node != DI_NODE_NIL) {
		if ((minor = di_minor_next(node, DI_MINOR_NIL)) != DI_MINOR_NIL) {
			instance = di_instance(node);
			phys_path = di_devfs_path(node);
			minor_name = di_minor_name(minor);
			strcpy(tmpnode, alias);
			sprintf(tmpnode, "%s%d", tmpnode, instance);
			strcpy(file, "/devices");
			strcat(file, phys_path);
			strcat(file, ":");
			strcat(file, minor_name);
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
	return (-1);
}

void build_mapping(){
	char device_name[512];
	int x;
	kstat_ctl_t *kc;
	kstat_t *ksp;
	kstat_io_t kios;

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
			get_alias(device_name);
		}
	}

	return;
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

long long get_ll_match(char *line, regmatch_t *match){
	char *ptr;
	long long num;

	ptr=line+match->rm_so;
	num=atoll(ptr);

	return num;
}

#ifdef ALLBSD
kvm_t *get_kvm() {
	static kvm_t *kvmd = NULL;

	if (kvmd != NULL) {
		return kvmd;
	}

	kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL);
	return kvmd;
}
#endif

#ifdef NETBSD
struct uvmexp *get_uvmexp() {
	static u_long addr = 0;
	static struct uvmexp uvm;
	kvm_t *kvmd = get_kvm();

	if (kvmd == NULL) {
		return NULL;
	}

	if (addr == 0) {
		struct nlist symbols[] = {
			{ "_uvmexp" },
			{ NULL }
		};

		if (kvm_nlist(kvmd, symbols) != 0) {
			return NULL;
		}
		addr = symbols[0].n_value;
	}

	if (kvm_read(kvmd, addr, &uvm, sizeof uvm) != sizeof uvm) {
		return NULL;
	}
	return &uvm;
}
#endif

int statgrab_init(){
#ifdef ALLBSD 
	{ 
		kvm_t *kvmd = get_kvm(); 
		if (kvmd == NULL) return 1;
	}
#endif
#ifdef NETBSD
	{
		struct uvmexp *uvm = get_uvmexp();
		if (uvm == NULL) return 1;
	}
#endif
#ifdef SOLARIS
	build_mapping();
#endif
	return 0;
}
