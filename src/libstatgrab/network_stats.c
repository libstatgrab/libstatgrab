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
#include <string.h>
#include "statgrab.h"
#include "time.h"
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#endif
#ifdef LINUX
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include "tools.h"
#endif

static network_stat_t *network_stats=NULL;
static int interfaces=0;

void network_stat_init(int start, int end, network_stat_t *net_stats){

	for(net_stats+=start; start<end; start++){
		net_stats->interface_name=NULL;
		net_stats->tx=0;
		net_stats->rx=0;
		net_stats++;
	}
}

network_stat_t *network_stat_malloc(int needed_entries, int *cur_entries, network_stat_t *net_stats){

	if(net_stats==NULL){

		if((net_stats=malloc(needed_entries * sizeof(network_stat_t)))==NULL){
			return NULL;
		}
		network_stat_init(0, needed_entries, net_stats);
		*cur_entries=needed_entries;

		return net_stats;
	}


	if(*cur_entries<needed_entries){
		net_stats=realloc(net_stats, (sizeof(network_stat_t)*needed_entries));
		if(net_stats==NULL){
			return NULL;
		}
		network_stat_init(*cur_entries, needed_entries, net_stats);
		*cur_entries=needed_entries;
	}

	return net_stats;
}


network_stat_t *get_network_stats(int *entries){

	static int sizeof_network_stats=0;	
	network_stat_t *network_stat_ptr;

#ifdef SOLARIS
        kstat_ctl_t *kc;
        kstat_t *ksp;
	kstat_named_t *knp;
#endif

#ifdef LINUX
	FILE *f;
	/* Horrible big enough, but it should be quite easily */
	char line[8096];
	regex_t regex;
	regmatch_t line_match[4];
#endif

#ifdef SOLARIS
        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

	interfaces=0;

    	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        	if (!strcmp(ksp->ks_class, "net")) {
                	kstat_read(kc, ksp, NULL);

			if((knp=kstat_data_lookup(ksp, "rbytes64"))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}

			network_stats=network_stat_malloc((interfaces+1), &sizeof_network_stats, network_stats);
			if(network_stats==NULL){
				return NULL;
			}
			network_stat_ptr=network_stats+interfaces;
			network_stat_ptr->rx=knp->value.ui64;

			if((knp=kstat_data_lookup(ksp, "obytes64"))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}
			network_stat_ptr->tx=knp->value.ui64;
			if(network_stat_ptr->interface_name!=NULL){
				free(network_stat_ptr->interface_name);
			}
			network_stat_ptr->interface_name=strdup(ksp->ks_name);

			network_stat_ptr->systime=time(NULL);
			interfaces++;
		}
	}
		
	kstat_close(kc);	
#endif
#ifdef LINUX
	f=fopen("/proc/net/dev", "r");
	if(f==NULL){
		return NULL;
	}
	/* read the 2 lines.. Its the title, so we dont care :) */
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);


	if((regcomp(&regex, "^[[:space:]]*([^:]+):[[:space:]]*([[:digit:]]+)[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+[[:digit:]]+[[:space:]]+([[:digit:]]+)", REG_EXTENDED))!=0){
		return NULL;
	}

	interfaces=0;

	while((fgets(line, sizeof(line), f)) != NULL){
		if((regexec(&regex, line, 4, line_match, 0))!=0){
			continue;
		}
        	network_stats=network_stat_malloc((interfaces+1), &sizeof_network_stats, network_stats);
       		if(network_stats==NULL){
      			return NULL;
		}
	        network_stat_ptr=network_stats+interfaces;

		if(network_stat_ptr->interface_name!=NULL){
			free(network_stat_ptr->interface_name);
		}

		network_stat_ptr->interface_name=get_string_match(line, &line_match[1]);
		network_stat_ptr->rx=get_ll_match(line, &line_match[2]);
		network_stat_ptr->tx=get_ll_match(line, &line_match[3]);
		network_stat_ptr->systime=time(NULL);

		interfaces++;
	}

#endif
	*entries=interfaces;

	return network_stats;	
}

network_stat_t *get_network_stats_diff(int *entries){
	static network_stat_t *network_stats_diff=NULL;
	static int sizeof_net_stats_diff=0;
	network_stat_t *network_stats_ptr, *network_stats_diff_ptr;
	int ifaces, x, y;

	if(network_stats==NULL){
		network_stats_ptr=get_network_stats(&ifaces);
		*entries=ifaces;
		return network_stats_ptr;
	}

	network_stats_diff=network_stat_malloc(interfaces, &sizeof_net_stats_diff, network_stats_diff);
	if(network_stats_diff==NULL){
		return NULL;
	}

	network_stats_ptr=network_stats;
	network_stats_diff_ptr=network_stats_diff;

	for(ifaces=0;ifaces<interfaces;ifaces++){
		if(network_stats_diff_ptr->interface_name!=NULL){
			free(network_stats_diff_ptr->interface_name);
		}
		network_stats_diff_ptr->interface_name=strdup(network_stats_ptr->interface_name);
		network_stats_diff_ptr->tx=network_stats_ptr->tx;
		network_stats_diff_ptr->rx=network_stats_ptr->rx;
		network_stats_diff_ptr->systime=network_stats->systime;

		network_stats_ptr++;
		network_stats_diff_ptr++;
	}
	network_stats_ptr=get_network_stats(&ifaces);		
	network_stats_diff_ptr=network_stats_diff;

	for(x=0;x<sizeof_net_stats_diff;x++){

		if((strcmp(network_stats_diff_ptr->interface_name, network_stats_ptr->interface_name))==0){
			network_stats_diff_ptr->tx = network_stats_ptr->tx - network_stats_diff_ptr->tx;
			network_stats_diff_ptr->rx = network_stats_ptr->rx - network_stats_diff_ptr->rx;	
			network_stats_diff_ptr->systime = network_stats_ptr->systime - network_stats_diff_ptr->systime;	
		}else{
			
			network_stats_ptr=network_stats;
			for(y=0;y<ifaces;y++){
				if((strcmp(network_stats_diff_ptr->interface_name, network_stats_ptr->interface_name))==0){
					network_stats_diff_ptr->tx = network_stats_ptr->tx - network_stats_diff_ptr->tx;
					network_stats_diff_ptr->rx = network_stats_ptr->rx - network_stats_diff_ptr->rx;	
					network_stats_diff_ptr->systime = network_stats_ptr->systime - network_stats_diff_ptr->systime;	
					break;
				}

				network_stats_ptr++;
			}	
		}

		network_stats_ptr++;
		network_stats_diff_ptr++;
	}

	*entries=sizeof_net_stats_diff;
	return network_stats_diff;
}	

