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

#ifdef HAVE_ATOLL
long long get_ll_match(char *line, regmatch_t *match){
	char *ptr;
	long long num;

	ptr=line+match->rm_so;
	num=atoll(ptr);

	return num;
}
#endif

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
	kvm_t *kvmd;
#endif
#ifdef NETBSD
	struct uvmexp *uvm;
#endif

#ifdef ALLBSD
	kvmd = get_kvm();
	if (kvmd == NULL) return 1;
#endif
#ifdef NETBSD
	uvm = get_uvmexp();
	if (uvm == NULL) return 1;
#endif
	return 0;
}
