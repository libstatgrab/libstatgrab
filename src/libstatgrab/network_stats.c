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
#include <string.h>
#include "statgrab.h"
#include <time.h>
#ifdef SOLARIS
#include <kstat.h>
#include <sys/sysinfo.h>
#endif
#ifdef LINUX
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ctype.h>
#include "tools.h"
/* Stuff which could be defined by defining KERNEL, but 
 * that would be a bad idea, so we'll just declare it here
 */
typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <unistd.h>
#endif
#ifdef ALLBSD
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_media.h>
#include <sys/ioctl.h>
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
	/* Horrible big enough, but it should be easily big enough */
	char line[8096];
	regex_t regex;
	regmatch_t line_match[4];
#endif
#ifdef ALLBSD
	struct ifaddrs *net, *net_ptr;
	struct if_data *net_data;
#endif

#ifdef ALLBSD
	if(getifaddrs(&net) != 0){
		return NULL;
	}

	interfaces=0;
	
	for(net_ptr=net;net_ptr!=NULL;net_ptr=net_ptr->ifa_next){
		if(net_ptr->ifa_addr->sa_family != AF_LINK) continue;
		network_stats=network_stat_malloc((interfaces+1), &sizeof_network_stats, network_stats);
		if(network_stats==NULL){
			return NULL;
		}
		network_stat_ptr=network_stats+interfaces;
		
		if(network_stat_ptr->interface_name!=NULL) free(network_stat_ptr->interface_name);
		network_stat_ptr->interface_name=strdup(net_ptr->ifa_name);
		if(network_stat_ptr->interface_name==NULL) return NULL;
		net_data=(struct if_data *)net_ptr->ifa_data;
		network_stat_ptr->rx=net_data->ifi_ibytes;
		network_stat_ptr->tx=net_data->ifi_obytes;			
		network_stat_ptr->systime=time(NULL);
		interfaces++;
	}
	freeifaddrs(net);	
#endif

#ifdef SOLARIS
        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

	interfaces=0;

    	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        	if (!strcmp(ksp->ks_class, "net")) {
                	kstat_read(kc, ksp, NULL);

#ifdef SOL7
#define RLOOKUP "rbytes"
#define WLOOKUP "obytes"
#define VALTYPE value.ui32
#else
#define RLOOKUP "rbytes64"
#define WLOOKUP "obytes64"
#define VALTYPE value.ui64
#endif

			if((knp=kstat_data_lookup(ksp, RLOOKUP))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}

			network_stats=network_stat_malloc((interfaces+1), &sizeof_network_stats, network_stats);
			if(network_stats==NULL){
				return NULL;
			}
			network_stat_ptr=network_stats+interfaces;
			network_stat_ptr->rx=knp->VALTYPE;

			if((knp=kstat_data_lookup(ksp, WLOOKUP))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}
			network_stat_ptr->tx=knp->VALTYPE;
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
	fclose(f);
	regfree(&regex);

#endif

#ifdef CYGWIN
	return NULL;
#endif

	*entries=interfaces;

	return network_stats;	
}

long long transfer_diff(long long new, long long old){
#if defined(SOL7) || defined(LINUX) || defined(FREEBSD)
#define MAXVAL 4294967296LL
#else
#define MAXVAL 18446744073709551616LL
#endif
	long long result;
	if(new>=old){
		result = (new-old);
	}else{
		result = (MAXVAL+(new-old));
	}

	return result;

}

network_stat_t *get_network_stats_diff(int *entries) {
	static network_stat_t *diff = NULL;
	static int diff_count = 0;
	network_stat_t *src, *dest;
	int i, j, new_count;

	if (network_stats == NULL) {
		/* No previous stats, so we can't calculate a difference. */
		return get_network_stats(entries);
	}

	/* Resize the results array to match the previous stats. */
	diff = network_stat_malloc(interfaces, &diff_count, diff);
	if (diff == NULL) {
		return NULL;
	}

	/* Copy the previous stats into the result. */
	for (i = 0; i < diff_count; i++) {
		src = &network_stats[i];
		dest = &diff[i];

		if (dest->interface_name != NULL) {
			free(dest->interface_name);
		}
		dest->interface_name = strdup(src->interface_name);
		dest->rx = src->rx;
		dest->tx = src->tx;
		dest->systime = src->systime;
	}

	/* Get a new set of stats. */
	if (get_network_stats(&new_count) == NULL) {
		return NULL;
	}

	/* For each previous stat... */
	for (i = 0; i < diff_count; i++) {
		dest = &diff[i];

		/* ... find the corresponding new stat ... */
		for (j = 0; j < new_count; j++) {
			/* Try the new stat in the same position first,
			   since that's most likely to be it. */
			src = &network_stats[(i + j) % new_count];
			if (strcmp(src->interface_name, dest->interface_name) == 0) {
				break;
			}
		}
		if (j == new_count) {
			/* No match found. */
			continue;
		}

		/* ... and subtract the previous stat from it to get the
		   difference. */
		dest->rx = transfer_diff(src->rx, dest->rx);
		dest->tx = transfer_diff(src->tx, dest->tx);
		dest->systime = src->systime - dest->systime;
	}

	*entries = diff_count;
	return diff;
}
/* NETWORK INTERFACE STATS */

void network_iface_stat_init(int start, int end, network_iface_stat_t *net_stats){

	for(net_stats+=start; start<end; start++){
		net_stats->interface_name=NULL;
		net_stats->speed=0;
		net_stats->dup=UNKNOWN_DUPLEX;
		net_stats++;
	}
}

network_iface_stat_t *network_iface_stat_malloc(int needed_entries, int *cur_entries, network_iface_stat_t *net_stats){

	if(net_stats==NULL){

		if((net_stats=malloc(needed_entries * sizeof(network_iface_stat_t)))==NULL){
			return NULL;
		}
		network_iface_stat_init(0, needed_entries, net_stats);
		*cur_entries=needed_entries;

		return net_stats;
	}


	if(*cur_entries<needed_entries){
		net_stats=realloc(net_stats, (sizeof(network_iface_stat_t)*needed_entries));
		if(net_stats==NULL){
			return NULL;
		}
		network_iface_stat_init(*cur_entries, needed_entries, net_stats);
		*cur_entries=needed_entries;
	}

	return net_stats;
}

network_iface_stat_t *get_network_iface_stats(int *entries){
	static network_iface_stat_t *network_iface_stats;
	network_iface_stat_t *network_iface_stat_ptr;
	static int sizeof_network_iface_stats=0;	
	static int ifaces;

#ifdef SOLARIS
        kstat_ctl_t *kc;
        kstat_t *ksp;
	kstat_named_t *knp;
#endif
#ifdef ALLBSD
        struct ifaddrs *net, *net_ptr;
	struct ifmediareq ifmed;
	struct ifreq ifr;
	int sock;
	int x;
#endif
#ifdef LINUX
        FILE *f;
        /* Horrible big enough, but it should be easily big enough */
        char line[8096];
	int sock;
#endif
	ifaces = 0;
#ifdef ALLBSD
        if(getifaddrs(&net) != 0){
                return NULL;
        }

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) return NULL;

	for(net_ptr=net; net_ptr!=NULL; net_ptr=net_ptr->ifa_next){
                if(net_ptr->ifa_addr->sa_family != AF_LINK) continue;
                network_iface_stats=network_iface_stat_malloc((ifaces+1), &sizeof_network_iface_stats, network_iface_stats);
                if(network_iface_stats==NULL){
                        return NULL;
                }
                network_iface_stat_ptr = network_iface_stats + ifaces;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, net_ptr->ifa_name, sizeof(ifr.ifr_name));

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0){
			continue;
		}	
		if((ifr.ifr_flags & IFF_UP) != 0){
			network_iface_stat_ptr->up = 1;
		}else{
			network_iface_stat_ptr->up = 0;
		}

		if (network_iface_stat_ptr->interface_name != NULL) free(network_iface_stat_ptr->interface_name);
		network_iface_stat_ptr->interface_name = strdup(net_ptr->ifa_name);
		if (network_iface_stat_ptr->interface_name == NULL) return NULL;

		network_iface_stat_ptr->speed = 0;
		network_iface_stat_ptr->dup = UNKNOWN_DUPLEX;
		ifaces++;

		memset(&ifmed, 0, sizeof(struct ifmediareq));
		strlcpy(ifmed.ifm_name, net_ptr->ifa_name, sizeof(ifmed.ifm_name));
		if(ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifmed) == -1){
			/* Not all interfaces support the media ioctls. */
			continue;
		}

		/* We may need to change this if we start doing wireless devices too */
		if( (ifmed.ifm_active | IFM_ETHER) != ifmed.ifm_active ){
			/* Not a ETHER device */
			continue;
		}

		/* Only intrested in the first 4 bits)  - Assuming only ETHER devices */
		x = ifmed.ifm_active & 0x0f;	
		switch(x){
			/* 10 Mbit connections. Speedy :) */
			case(IFM_10_T):
			case(IFM_10_2):
			case(IFM_10_5):
			case(IFM_10_STP):
			case(IFM_10_FL):
				network_iface_stat_ptr->speed = 10;
				break;
			/* 100 Mbit conneections */
			case(IFM_100_TX):
			case(IFM_100_FX):
			case(IFM_100_T4):
			case(IFM_100_VG):
			case(IFM_100_T2):
				network_iface_stat_ptr->speed = 100;
				break;
			/* 1000 Mbit connections */
			case(IFM_1000_SX):
			case(IFM_1000_LX):
			case(IFM_1000_CX):
#if defined(FREEBSD) && !defined(FREEBSD5)
			case(IFM_1000_TX):
			case(IFM_1000_FX):
#else
			case(IFM_1000_T):
#endif
				network_iface_stat_ptr->speed = 1000;
				break;
			/* We don't know what it is */
			default:
				network_iface_stat_ptr->speed = 0;
				break;
		}

		if( (ifmed.ifm_active | IFM_FDX) == ifmed.ifm_active ){
			network_iface_stat_ptr->dup = FULL_DUPLEX;
		}else if( (ifmed.ifm_active | IFM_HDX) == ifmed.ifm_active ){
			network_iface_stat_ptr->dup = HALF_DUPLEX;
		}else{
			network_iface_stat_ptr->dup = UNKNOWN_DUPLEX;
		}

	}	
	freeifaddrs(net);
	close(sock);
#endif

#ifdef SOLARIS
        if ((kc = kstat_open()) == NULL) {
                return NULL;
        }

    	for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
        	if (!strcmp(ksp->ks_class, "net")) {
                	kstat_read(kc, ksp, NULL);
			if((knp=kstat_data_lookup(ksp, "ifspeed"))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}
			network_iface_stats=network_iface_stat_malloc((ifaces+1), &sizeof_network_iface_stats, network_iface_stats);
			if(network_iface_stats==NULL){
				return NULL;
			}
			network_iface_stat_ptr = network_iface_stats + ifaces;
			network_iface_stat_ptr->speed = knp->value.ui64 / (1000*1000);

                        if((knp=kstat_data_lookup(ksp, "link_up"))==NULL){
                                /* Not a network interface, so skip to the next entry */
                                continue;
                        }
			/* Solaris has 1 for up, and 0 for not. As we do too */
                        network_iface_stat_ptr->up = value.ui32;

			if((knp=kstat_data_lookup(ksp, "link_duplex"))==NULL){
				/* Not a network interface, so skip to the next entry */
				continue;
			}

			network_iface_stat_ptr->dup = UNKNOWN_DUPLEX;
			if(knp->value.ui32 == 2){
				network_iface_stat_ptr->dup = FULL_DUPLEX;
			}
			if(knp->value.ui32 == 1){
				network_iface_stat_ptr->dup = HALF_DUPLEX;
			}

			if(network_iface_stat_ptr->interface_name != NULL) free(network_iface_stat_ptr->interface_name);
			network_iface_stat_ptr->interface_name = strdup(ksp->ks_name);
			if(network_iface_stat_ptr->interface_name == NULL) return NULL;
			ifaces++;
		}
	}
	kstat_close(kc);
	
#endif	
#ifdef LINUX
	f = fopen("/proc/net/dev", "r");
        if(f == NULL){
                return NULL;
        }

	/* Setup stuff so we can do the ioctl to get the info */
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		return NULL;
	}

	/* Ignore first 2 lines.. Just headings */
        if((fgets(line, sizeof(line), f)) == NULL) return NULL;
        if((fgets(line, sizeof(line), f)) == NULL) return NULL;

        while((fgets(line, sizeof(line), f)) != NULL){
                char *name, *ptr;
                struct ifreq ifr;
                struct ethtool_cmd ethcmd;
                int err;

		/* Get the interface name */
                ptr = strchr(line, ':');
                if (ptr == NULL) continue;
                *ptr='\0';
                name = line;
                while(isspace(*(name))){
                        name++;
                }

                memset(&ifr, 0, sizeof ifr);
                strncpy(ifr.ifr_name, name, sizeof ifr.ifr_name);

		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
			continue;
		}

		/* We have a good interface to add */
		network_iface_stats=network_iface_stat_malloc((ifaces+1), &sizeof_network_iface_stats, network_iface_stats);
		if(network_iface_stats==NULL){
			return NULL;
		}
		network_iface_stat_ptr = network_iface_stats + ifaces;
		network_iface_stat_ptr->interface_name = strdup(name);
		if ((ifr.ifr_flags & IFF_UP) != 0) {
			network_iface_stat_ptr->up = 1;
		} else {
			network_iface_stat_ptr->up = 0;
		}

                memset(&ethcmd, 0, sizeof ethcmd);
                ethcmd.cmd = ETHTOOL_GSET;
                ifr.ifr_data = (caddr_t) &ethcmd;

                err = ioctl(sock, SIOCETHTOOL, &ifr);
                if (err == 0) {
			network_iface_stat_ptr->speed = ethcmd.speed;

			switch (ethcmd.duplex) {
			case 0x00:
				network_iface_stat_ptr->dup = FULL_DUPLEX;
				break;
			case 0x01:
				network_iface_stat_ptr->dup = HALF_DUPLEX;
				break;
			default:
				network_iface_stat_ptr->dup = UNKNOWN_DUPLEX;
			}
		} else {
			/* Not all interfaces support the ethtool ioctl. */
			network_iface_stat_ptr->speed = 0;
			network_iface_stat_ptr->dup = UNKNOWN_DUPLEX;
		}

		ifaces++;
	}
	close(sock);
	fclose(f);
#endif
	*entries = ifaces;
	return network_iface_stats; 
}

