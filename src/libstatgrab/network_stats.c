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

#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#include <string.h>
#endif

#define START_VAL 1

network_stat_t *network_stat_init(int num_iface, int *wmark, network_stat_t *net_stats){
	int x;
	network_stat_t *net_stats_ptr;

	if(*wmark==-1){
		printf("new malloc\n");
		if((net_stats=malloc(START_VAL * sizeof(network_stat_t)))==NULL){
			return NULL;
		}
		*wmark=START_VAL;
		net_stats_ptr=net_stats;
		for(x=0;x<(*wmark);x++){
			net_stats_ptr->interface_name=NULL;
			net_stats_ptr++;
		}
		return net_stats;
	}
	if(num_iface>(*wmark-1)){
		if((net_stats=realloc(net_stats, (*wmark)*2 * sizeof(network_stat_t)))==NULL){
			return NULL;
		}

		*wmark=(*wmark)*2;
		net_stats_ptr=net_stats+num_iface;
		for(x=num_iface;x<(*wmark);x++){
			net_stats_ptr->interface_name=NULL;
			net_stats_ptr++;
		}
		return net_stats;
	}

	return net_stats;
}

network_stat_t *get_network_stats(int *entries){
        kstat_ctl_t *kc;
        kstat_t *ksp;
	kstat_named_t *knp;
	
	int interfaces=0;	
	network_stat_t *network_stat_ptr;
	static network_stat_t *network_stats=NULL;
	static int watermark=-1;

        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

    	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        	if (!strcmp(ksp->ks_class, "net")) {
                	kstat_read(kc, ksp, NULL);

			if((knp=kstat_data_lookup(ksp, "rbytes64"))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}
			network_stats=network_stat_init(interfaces, &watermark, network_stats);
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
			interfaces++;
		}
	}
		
	kstat_close(kc);	

	*entries=interfaces;

	return network_stats;	

}

